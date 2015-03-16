#pragma once

#include "ClientConnection.h"
#include "Typedefs.h"
#include "Package.h"

namespace AsyncServer
{
    typedef cpp11::shared_ptr<ClientConnection> ClientConnectionPtr;

    class Server: boost::noncopyable
    {
    private:

        unsigned int m_serverPort;

        asio::io_service& m_ioService;

        asio::ip::tcp::acceptor m_acceptor;

        std::deque<cpp11::shared_ptr<Package> > m_tasksQueue;

        cpp11::mutex m_mutexQueue;

        void StartAccept();

        void HandleAccept(const boost::system::error_code& errorCode, ClientConnectionPtr connection);

        void HandleWrite(const boost::system::error_code& errorCode, ClientConnectionPtr connection);

        void PushQueue(cpp11::shared_ptr<Package>);

        cpp11::shared_ptr<Package> PopQueue();

        bool IsQueueEmpty();

        unsigned int GetQueueSize();

    public:

        Server(asio::io_service&, unsigned int);

        void AddTask(cpp11::shared_ptr<Package>);
    };
}