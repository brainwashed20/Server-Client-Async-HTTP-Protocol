#include "Requirements.h"

#include "Server.h"
#include "Utils.h"

namespace AsyncServer
{
    Server::Server(boost::asio::io_service& io_service, unsigned int port) :
        m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        m_ioService(io_service)
    {
        std::cout << "Listening to TCP socket on port " << port << "..." << std::endl;

        StartAccept();
    }

    void Server::StartAccept()
    {
        ClientConnectionPtr connection(new ClientConnection(m_acceptor.get_io_service()));

        m_acceptor.async_accept(
            connection->getSocket(),
            boost::bind(
                &Server::HandleAccept,
                this,
                boost::asio::placeholders::error,
                connection
            )
        );
    }

    void Server::HandleAccept(const boost::system::error_code& errorCode, ClientConnectionPtr connection)
    {
        if(!errorCode)
        {
            // Successfully accepted a new connection. The connection::async_write() function will automatically serialize the data structure for us.

            cpp11::shared_ptr<Package> packageToSend;
            PushQueue(packageToSend);
            if(packageToSend != nullptr)
            {
                connection->async_write(
                    *packageToSend,
                    boost::bind(
                        &Server::HandleWrite, 
                        this,
                        boost::asio::placeholders::error, 
                        connection
                    )
                );
            }

            // start accepting the next client
            StartAccept();
        }
        else
        {
            std::cout << "Error: " << errorCode.message() << std::endl;
        }
    }

    void Server::HandleWrite(const boost::system::error_code& errorCode, ClientConnectionPtr connection)
    {

    }

    void Server::PushQueue(cpp11::shared_ptr<Package> msg)
    {
        std::unique_lock<std::mutex> queueLock(m_mutexQueue);
        m_tasksQueue.push_back(msg);
    }

    cpp11::shared_ptr<Package> Server::PopQueue()
    {
        cpp11::shared_ptr<Package> res;
        bool isEmpty = true;

        {
            std::unique_lock<std::mutex> queueLock(m_mutexQueue);
            if(!m_tasksQueue.empty())
            {
                res = m_tasksQueue.front();
                isEmpty = false;
                m_tasksQueue.pop_front();
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
            std::unique_lock<std::mutex> queueLock(m_mutexQueue);
            bool isEmpty = m_tasksQueue.empty();
        }

        return isEmpty;
    }

    void Server::AddTask(cpp11::shared_ptr<Package> package)
    {
        PushQueue(package);
    }
}