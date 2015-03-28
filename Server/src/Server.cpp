#include "Server.h"
#include "Utils.h"

namespace AsyncServer
{
    Server::Server(boost::asio::io_service& io_service, unsigned int port) :
        m_Acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        m_FinishedReadingChunks(false),
        m_TotalNumChunksRead(0),
        m_Ios(io_service)
    {
        std::cout << "Listening to TCP socket on port " << port << "..." << std::endl;

        StartAccept();
    }

    void Server::StartAccept()
    {
        ClientConnectionPtr connection(
            new ClientConnection(
                m_Acceptor.get_io_service()
            )
        );

        m_Acceptor.async_accept(
            connection->GetSocket(),
            boost::bind(
                &Server::HandleAccept,
                this,
                connection,
                boost::asio::placeholders::error
            )
        );
    }

    void Server::HandleAccept(ClientConnectionPtr connection, const boost::system::error_code& errorCode)
    {
        if(!errorCode)
        {
            connection->Start(this);

            // start accepting the next client
            StartAccept();
        }
        else
        {
            std::cout << "Error: " << errorCode.message() << std::endl;
        }
    }

    void Server::ProcessInputTgaFile(const std::string& filePath, const std::string& fileNameTga)
    {
        std::string fullPath(filePath + "\\" + fileNameTga);

        TgaHeader tgaHeader;

        std::ifstream myFile(fullPath, std::ios::binary | std::ios::in);

        if(myFile.is_open())
        {
            // read tga header
            myFile.read((char*) &tgaHeader, sizeof(tgaHeader));
            m_TgaHeaders.push_back(std::make_pair(tgaHeader, fileNameTga));

            // get image data size
            myFile.seekg(0, std::ios::end);
            unsigned int tgaImageSize = (unsigned int) myFile.tellg() - (unsigned int)sizeof(tgaHeader) - (unsigned int) tgaHeader.identsize;

            // start reading chunks
            myFile.seekg(sizeof(tgaHeader) + tgaHeader.identsize, std::ios_base::beg);

            unsigned int tgaBpp = tgaHeader.bits;

            unsigned int chunksNumber = tgaImageSize / 8400;
            unsigned int chunkSize = (((tgaImageSize / chunksNumber) + 11) / 12) * 12;

            PoolAllocCharVector buffer;
            buffer.reserve(chunkSize);

            std::vector<char> toQueue;
            toQueue.reserve(chunkSize + 100);

            int chunkIndex = -1;
            while(!myFile.eof())
            {
                {
                    std::unique_lock<std::mutex> readLock(m_ReadingThreadMutex);
                    
                    m_ReadingThreadCV.wait(readLock, [chunksNumber, this]
                        {
                            if(chunksNumber < 100)
                                return IsQueueEmpty();

                            return GetQueueSize() < 100;
                        }
                    );
                }

                while(!myFile.eof() && GetQueueSize() < 100)
                {
                    m_TotalNumChunksRead++;

                    chunksNumber--;

                    buffer.clear();
                    buffer.resize(chunkSize);
                    myFile.read(buffer.data(), chunkSize);

                    Utils::AppendString(buffer, fileNameTga);
                    Utils::EncodeInt(buffer, tgaBpp);
                    Utils::EncodeInt(buffer, ++chunkIndex);

                    toQueue.clear();
                    Utils::EncodeInt(toQueue, buffer.size());

                    std::copy(buffer.begin(), buffer.end(), std::back_inserter(toQueue));

                    auto queuePtr = std::make_shared<std::vector<char> >(toQueue);

                    PushQueue(queuePtr);
                }
            }

            std::cout << "Read " << chunkIndex << " chunks from " << fileNameTga << std::endl;

            myFile.close();
        }
    }

    void Server::ProcessInputTgaFolder(const std::string& inputDirectoryName, const std::string& outputDirectoryName)
    {
        try
        {
            if(boost::filesystem::exists(inputDirectoryName))
            {
                if(boost::filesystem::is_directory(inputDirectoryName))
                {
                    std::vector<boost::filesystem::path> directoryFiles;

                    // store the directory files
                    std::copy(
                        boost::filesystem::directory_iterator(inputDirectoryName),
                        boost::filesystem::directory_iterator(),
                        std::back_inserter(directoryFiles)
                    );

                    // remove from directory the files that are not TGA
                    for(size_t i = 0; i < directoryFiles.size(); ++i)
                    {
                        bool isTGA = directoryFiles[i].extension().generic_string().compare(".TGA") == 0 ||
                            directoryFiles[i].extension().generic_string().compare(".tga") == 0;

                        if(isTGA == false)
                        {
                            directoryFiles.erase(directoryFiles.begin() + i);
                        }
                    }

                    // process TGA files
                    for(size_t i = 0; i < directoryFiles.size(); ++i)
                    {
                        ProcessInputTgaFile(inputDirectoryName, directoryFiles[i].filename().generic_string());
                    }
                    m_FinishedReadingChunks = true;

                    {
                        // wait until processing tga's job is done
                        std::unique_lock<std::mutex> writeTgaLock(m_ReadingThreadMutex);
                        
                        m_ReadingThreadCV.wait(writeTgaLock, [this]
                            {
                                return IsJobFinished() == true;
                            }
                        );
                    }
                    WriteModifiedTgaFiles(outputDirectoryName);

                    // all server job is done
                    m_Ios.stop();
                }
                else
                {
                    std::cout << "Error: " << inputDirectoryName << " is not a directory!" << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "Error: " << inputDirectoryName << " doesn't exist!" << std::endl;
                return;
            }
        }
        catch(const boost::filesystem::filesystem_error & ex)
        {
            std::cout << "Error: " << ex.what() << std::endl;
            return;
        }
    }

    void Server::WriteModifiedTgaFiles(const std::string& outputDirectoryName)
    {
        std::sort(m_TgaHeaders.begin(), m_TgaHeaders.end(), Utils::CompareTgaHeaders);
        std::sort(m_OutputChunks.begin(), m_OutputChunks.end(), Utils::CompareOutputChunks);

        std::for_each(m_TgaHeaders.begin(), m_TgaHeaders.end(), [this, outputDirectoryName](std::pair<TgaHeader, std::string> tgaHeaderElem)
            {
                std::vector<char> outputImageData;
                int numChunks = -1;

                while(!m_OutputChunks.empty() && (std::get<0>(*m_OutputChunks.front()) == tgaHeaderElem.second))
                {
                    ++numChunks;

                    std::copy(
                        std::get<2>(*m_OutputChunks.front()).begin(),
                        std::get<2>(*m_OutputChunks.front()).end(),
                        std::back_inserter(outputImageData)
                    );

                    m_OutputChunks.pop_front();
                }

                std::string convertedFileFullPath(outputDirectoryName + "\\" + "new_" + tgaHeaderElem.second);

                std::ofstream outputFile(convertedFileFullPath, std::ios::binary | std::ios::out);
                outputFile.write((char*) &tgaHeaderElem.first, sizeof(tgaHeaderElem.first));
                outputFile.write(outputImageData.data(), outputImageData.size());
                outputFile.close();

                std::cout << "Job finished for " << tgaHeaderElem.second << "; Wrote " << numChunks << " chunks." << std::endl;
            }
        );

        std::cout << "All jobs finished!" << std::endl;
    }

    void Server::PushQueue(InitialChunkPtr msg)
    {
        std::unique_lock<std::mutex> queueLock(m_QueueMutex);
        m_InputChunks.push(msg);
    }

    InitialChunkPtr Server::PopQueue()
    {
        InitialChunkPtr res;
        bool isEmpty = true;

        {
            std::unique_lock<std::mutex> queueLock(m_QueueMutex);
            if(!m_InputChunks.empty())
            {
                res = m_InputChunks.front();
                isEmpty = false;
                m_InputChunks.pop();
            }
        }

        if(isEmpty == false)
        {
            return res;
        }

        return nullptr;
    }

    bool Server::IsQueueEmpty()
    {
        bool isEmpty = true;

        {
            std::unique_lock<std::mutex> queueLock(m_QueueMutex);
            bool isEmpty = m_InputChunks.empty();
        }

        return isEmpty;
    }

    bool Server::IsJobFinished()
    {
        return (m_FinishedReadingChunks && IsQueueEmpty() && (m_TotalNumChunksRead == GetOutputDataSize()));
    }

    void Server::InsertOutputChunk(ReceivedChunkPtr msg)
    {
        std::unique_lock<std::mutex> queueLock(m_QueueMutex);
        m_OutputChunks.push_back(msg);
    }

    void Server::NotifyReadingThread()
    {
        m_ReadingThreadCV.notify_one();
    }

    std::mutex& Server::GetReadingThreadMutex()
    {
        return m_ReadingThreadMutex;
    }

    unsigned int Server::GetQueueSize()
    {
        unsigned int qSize = 0;

        {
            std::unique_lock<std::mutex> queueLock(m_QueueMutex);
            qSize = m_InputChunks.size();
        }

        return qSize;
    }

    unsigned int Server::GetOutputDataSize()
    {
        unsigned int oSize = 0;

        {
            std::unique_lock<std::mutex> queueLock(m_QueueMutex);
            oSize = m_OutputChunks.size();
        }

        return oSize;
    }
}