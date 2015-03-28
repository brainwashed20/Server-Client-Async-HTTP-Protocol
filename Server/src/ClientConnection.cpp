#include "ClientConnection.h"
#include "Server.h"
#include "Utils.h"

namespace AsyncServer
{
    ClientConnection::ClientConnection(boost::asio::io_service& ios) :
        m_Socket(ios),
        m_HeaderData(k_HeaderSize),
        m_IsDisconnected(false),
        m_ReadQueueTimer(ios),
        m_ReadQueueDelay(500)
    {
        std::string str("empty");
        Utils::EncodeInt(m_EmptyMessage, str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(m_EmptyMessage));

        str.clear();
        str.assign("bye");
        Utils::EncodeInt(m_ByeMessage, str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(m_ByeMessage));
    }

    void ClientConnection::Start(ServerInstance server)
    {
        std::cout << m_Socket.remote_endpoint() << ": Connection accepted" << std::endl;

        StartSendingMessages(server);
    }

    void ClientConnection::StartSendingMessages(ServerInstance server)
    {
        if(m_IsDisconnected)
            return;

        InitialChunkPtr currentChunkPtr = server->PopQueue();

        if(currentChunkPtr != nullptr)
        {
            server->NotifyReadingThread();

            StartSendMessage(server, currentChunkPtr);
        }
        else
        {
            InitialChunkPtr myChunkSendPtr = std::make_shared<std::vector<char> >(m_EmptyMessage);

            m_ReadQueueTimer.expires_from_now(boost::posix_time::milliseconds(m_ReadQueueDelay));
            m_ReadQueueTimer.async_wait(
                boost::bind(
                    &ClientConnection::StartSendMessage, 
                    shared_from_this(), 
                    server, 
                    myChunkSendPtr
                )
            );
        }
    }


    void ClientConnection::StartSendMessage(ServerInstance server, InitialChunkPtr msg)
    {
        async_write(
            m_Socket,
            boost::asio::buffer(
                *msg,
                msg->size()
            ),
            boost::bind(
                &ClientConnection::HandleSendMessage,
                shared_from_this(),
                server,
                msg,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred
            )
        );
    }

    void ClientConnection::HandleSendMessage(ServerInstance server, InitialChunkPtr msg, const boost::system::error_code& errorCode, size_t size)
    {
        if(!errorCode)
        {
            async_read(
                m_Socket,
                boost::asio::buffer(
                    m_HeaderData,
                    k_HeaderSize
                ),
                boost::bind(
                    &ClientConnection::HandleReadHeader,
                    shared_from_this(),
                    server,
                    msg,
                    boost::asio::placeholders::error
                )
            );
        }
        else if(errorCode == boost::asio::error::eof)
        {
            std::cout << m_Socket.remote_endpoint() << ": Connection closed (handle sent)" << std::endl;
            m_Socket.close();
        }
        else
        {
            std::cout << m_Socket.remote_endpoint() << ": Client disconnected" << std::endl;
        }
    }

    void ClientConnection::HandleReadHeader(ServerInstance server, InitialChunkPtr msg, const boost::system::error_code& errorCode)
    {
        if(!errorCode)
        {
            m_BodySize = 0;
            Utils::DecodeHeader(m_HeaderData, m_BodySize);

            bool isNullMessage = (m_BodySize == 0);

            if(!isNullMessage)
            {
                m_BodyData.resize(m_BodySize);

                async_read(
                    m_Socket,
                    boost::asio::buffer(
                        m_BodyData,
                        m_BodySize
                    ),
                    boost::bind(
                        &ClientConnection::HandleReadBody,
                        shared_from_this(),
                        server,
                        msg,
                        boost::asio::placeholders::error
                    )
                );
            }
            else
            {
                server->PushQueue(msg);
                StartSendingMessages(server);
            }
        }
        else
        {
            server->PushQueue(msg);
            ClientDisconected();
            // std::cout << "Error receiving message header: " << errorCode.message() << std::endl;
        }
    }

    void ClientConnection::HandleReadBody(ServerInstance server, InitialChunkPtr msg, const boost::system::error_code& errorCode)
    {
        if(!errorCode)
        {
            bool isEmptyMessage = (m_BodySize == 5 && m_BodyData[0] == 'e' && m_BodyData[4] == 'y');

            if(!isEmptyMessage)
            {
                unsigned int chunkIndex, bpp;
                std::string currentFileNameTga;

                Utils::DecodeInt(m_BodyData, chunkIndex);
                Utils::DecodeInt(m_BodyData, bpp);
                Utils::GetString(m_BodyData, currentFileNameTga);

                ReceivedChunk res = std::make_tuple(currentFileNameTga, chunkIndex, m_BodyData);
                ReceivedChunkPtr resPtr = std::make_shared<ReceivedChunk>(res);

                server->InsertOutputChunk(resPtr);

                if(server->IsJobFinished())
                {
                    {
                        std::lock_guard<std::mutex> notifyLock(server->GetReadingThreadMutex());
                        InitialChunkPtr myChunkSendPtr = std::make_shared<std::vector<char> >(m_ByeMessage);
                        StartSendMessage(server, myChunkSendPtr);
                    }
                    server->NotifyReadingThread();
                }
                else
                {
                    StartSendingMessages(server);
                }
            }
            else
            {
                StartSendingMessages(server);
            }
        }
        else
        {
            ClientDisconected();
            // std::cout << "Error receiving message body: " << errorCode.message() << std::endl;
        }
    }

    void ClientConnection::ClientDisconected()
    {
        m_IsDisconnected = true;
    }

    boost::asio::ip::tcp::socket& ClientConnection::GetSocket()
    {
        return m_Socket;
    }
}