#pragma once

#include "Typedefs.h"

namespace AsyncServer
{
    class Server;

    typedef Server* ServerInstance;

    class ClientConnection :
        public std::enable_shared_from_this <ClientConnection>
    {
    private:

        static const unsigned int k_HeaderSize = 4;

        boost::asio::ip::tcp::socket m_Socket;

        boost::asio::deadline_timer m_ReadQueueTimer;

        std::vector<char> m_HeaderData;
        std::vector<char> m_BodyData;
        std::vector<char> m_EmptyMessage;
        std::vector<char> m_ByeMessage;

        unsigned int m_BodySize;
        unsigned int m_ReadQueueDelay;

        bool m_IsDisconnected;

    public:

        ClientConnection(boost::asio::io_service& ios);

        void Start(ServerInstance server);

        void StartSendingMessages(ServerInstance server);

        void StartSendMessage(ServerInstance server, InitialChunkPtr);

        void HandleSendMessage(ServerInstance server, InitialChunkPtr, const boost::system::error_code& errorCode, size_t size);

        void HandleReadHeader(ServerInstance server, InitialChunkPtr, const boost::system::error_code& errorCode);

        void HandleReadBody(ServerInstance server, InitialChunkPtr, const boost::system::error_code& errorCode);

        void ClientDisconected();

        boost::asio::ip::tcp::socket& GetSocket();
    };
}