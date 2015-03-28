#pragma once

#include "Includes.h"
#include "Utils.h"
#include "Typedefs.h"

namespace AsyncClient
{
    class Client
    {
    private:

        static const unsigned int k_HeaderSize = 4;

        boost::asio::io_service& m_Ios;

        boost::asio::ip::tcp::endpoint m_EndPoint;
        boost::asio::ip::tcp::socket m_Socket;

        bool m_Stop;

        std::vector<char> m_HeaderData;
        std::vector<char> m_BodyData;
        std::vector<char> m_EmptyMessage;

        unsigned int m_BodySize;

        MessageQueue m_MessageQueue;

    public:
        Client(boost::asio::io_service& ios);

        void Start(boost::asio::ip::tcp::resolver::iterator it);

        void StartConnect(boost::asio::ip::tcp::resolver::iterator it);

        void HandleConnect(boost::asio::ip::tcp::resolver::iterator it, const boost::system::error_code& ec);

        void StartReadHeader();

        void HandleReadHeader(const boost::system::error_code& ec);

        void HandleReadBody(const boost::system::error_code& ec);

        void StartSendMessage(MessagePtr msg);

        void HandleSendMessage(MessagePtr msg, const boost::system::error_code& ec, size_t size);

        bool SendMessage(MessagePtr msg);

        void DoStop();

        void Stop();

        ~Client();
    };
}