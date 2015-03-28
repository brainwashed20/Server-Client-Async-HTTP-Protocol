#pragma once

#include "ClientConnection.h"
#include "Typedefs.h"
#include "TgaHeader.h"

namespace AsyncServer
{
    typedef std::shared_ptr<ClientConnection> ClientConnectionPtr;

    class Server
    {
    private:

        boost::asio::io_service& m_Ios;

        boost::asio::ip::tcp::acceptor m_Acceptor;

        InitialChunks m_InputChunks;

        ReceivedChunks m_OutputChunks;

        std::mutex m_QueueMutex, m_ReadingThreadMutex;

        std::condition_variable m_ReadingThreadCV;

        std::atomic<bool> m_FinishedReadingChunks;

        std::vector<std::pair<TgaHeader, std::string> > m_TgaHeaders;

        unsigned int m_TotalNumChunksRead;

    public:

        Server(boost::asio::io_service&, unsigned int);

        void StartAccept();

        void HandleAccept(ClientConnectionPtr, const boost::system::error_code&);

        void ProcessInputTgaFile(const std::string&, const std::string&);

        void ProcessInputTgaFolder(const std::string&, const std::string&);

        void WriteModifiedTgaFiles(const std::string&);

        void PushQueue(InitialChunkPtr);

        InitialChunkPtr PopQueue();

        bool IsQueueEmpty();

        void InsertOutputChunk(ReceivedChunkPtr);

        bool IsJobFinished();

        void NotifyReadingThread();

        unsigned int GetQueueSize();

        unsigned int GetOutputDataSize();

        std::mutex& GetReadingThreadMutex();
    };
}