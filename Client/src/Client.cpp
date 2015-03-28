#include "Client.h"

namespace AsyncClient
{
    Client::Client(boost::asio::io_service &ios) :
        m_Ios(ios),
        m_Socket(ios),
        m_Stop(false),
        m_MessageQueue(),
        m_HeaderData(k_HeaderSize)
    {
        std::string str("empty");
        Utils::EncodeInt(m_EmptyMessage, str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(m_EmptyMessage));
    }

    void Client::Start(boost::asio::ip::tcp::resolver::iterator it)
    {
        StartConnect(it);
    }

    void Client::StartConnect(boost::asio::ip::tcp::resolver::iterator it)
    {
        //is there an endpoint ?
        if(it != boost::asio::ip::tcp::resolver::iterator())
        {
            m_EndPoint = it->endpoint();

            std::cout << "Connecting to " << m_EndPoint << "..." << std::endl;

            m_Socket.async_connect(
                *it,
                boost::bind(
                    &Client::HandleConnect,
                    this,
                    it,
                    boost::asio::placeholders::error
                )
            );
        }
        else
        {
            DoStop();
        }
    }

    void Client::HandleConnect(boost::asio::ip::tcp::resolver::iterator it, const boost::system::error_code &errorCode)
    {
        if(m_Stop)
            return;

        if(!m_Socket.is_open())
        {
            std::cout << "Connect error: " << errorCode.message() << std::endl;
            m_Socket.close();
            StartConnect(++it);
            return;
        }

        std::cout << "Connected to " << it->endpoint() << std::endl;

        // start reading messages asynchonously
        StartReadHeader();
    }

    void Client::StartReadHeader()
    {
        if(m_Stop)
        {
            return;
        }

        async_read(
            m_Socket,
            boost::asio::buffer(
                m_HeaderData,
                k_HeaderSize
            ),
                boost::bind(
                &Client::HandleReadHeader,
                this,
                boost::asio::placeholders::error
            )
        );
    }

    void Client::HandleReadHeader(const boost::system::error_code &errorCode)
    {
        if(m_Stop)
        {
            return;
        }

        if(!errorCode)
        {
            m_BodySize = 0;
            Utils::DecodeHeader(m_HeaderData, m_BodySize);

            if(m_BodySize == 0)
            {
                StartReadHeader();
            }
            else
            {
                m_BodyData.resize(m_BodySize);

                async_read(
                    m_Socket,
                    boost::asio::buffer(
                        m_BodyData,
                        m_BodySize
                    ),
                    boost::bind(
                        &Client::HandleReadBody,
                        this,
                        boost::asio::placeholders::error
                    )
                );
            }
        }
        else
        {
            std::cout << "Error receiving message header: " << errorCode.message() << std::endl;
            DoStop();
        }
    }

    void Client::HandleReadBody(const boost::system::error_code &errorCode)
    {
        if(!errorCode)
        {
            bool isByeMessage = (m_BodySize == 3 && m_BodyData[0] == 'b' && m_BodyData[1] == 'y' &&  m_BodyData[2] == 'e');
            bool isEmptyMessage = (m_BodySize == 5 && m_BodyData[0] == 'e' && m_BodyData[4] == 'y');

            if(!isByeMessage && !isEmptyMessage)
            {
                // get the index and the filename out
                unsigned int chunkIndex, tgaBpp;
                std::string currentFileNameTga;

                std::vector<char> receivedMessage = m_BodyData;

                Utils::DecodeInt(receivedMessage, chunkIndex);
                Utils::DecodeInt(receivedMessage, tgaBpp);
                Utils::GetString(receivedMessage, currentFileNameTga);

                std::cout << currentFileNameTga << ": Received chunk " << chunkIndex << std::endl;

                // convert pixel to black and white

                std::vector<char> convertedChunk = receivedMessage;
                Utils::ConvertTgaChunkToBlackAndWhite(convertedChunk, tgaBpp);

                Utils::AppendString(convertedChunk, currentFileNameTga);
                Utils::EncodeInt(convertedChunk, tgaBpp);
                Utils::EncodeInt(convertedChunk, chunkIndex);

                std::vector<char> toSend;
                Utils::EncodeInt(toSend, convertedChunk.size());
                std::copy(convertedChunk.begin(), convertedChunk.end(), std::back_inserter(toSend));

                MessagePtr myMessagePtr;
                myMessagePtr = std::make_shared<std::vector<char> >(toSend);

                StartSendMessage(myMessagePtr);
            }
            else if(isEmptyMessage)
            {
                MessagePtr myMessagePtr = std::make_shared<std::vector<char> >(m_EmptyMessage);
                StartSendMessage(myMessagePtr);
            }
            else
            {
                std::cout << "Job finished!" << std::endl;
                DoStop();
            }
        }
        else
        {
            std::cout << "Error receiving message body: " << errorCode.message() << std::endl;
            DoStop();
        }
    }

    void Client::StartSendMessage(MessagePtr msg)
    {
        async_write(
            m_Socket,
            boost::asio::buffer(
                *msg,
                msg->size()
            ),
            boost::bind(
                &Client::HandleSendMessage,
                this,
                msg,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred
            )
        );
    }

    void Client::HandleSendMessage(MessagePtr msg, const boost::system::error_code &errorCode, size_t size)
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
                    &Client::HandleReadHeader,
                    this,
                    boost::asio::placeholders::error
                )
            );

        }
        else
        {
            std::cout << "Error sending message: " << errorCode.message() << std::endl;
            DoStop();
            return;
        }
    }

    // called from the main thread since the io service runs in its own thread, the send is scheduled in the IO service
    bool Client::SendMessage(MessagePtr msg)
    {
        if(m_BodySize == 0)
        {
            return false;
        }

        if(!m_Stop)
        {
            m_Ios.post(boost::bind(&Client::StartSendMessage, this, msg));
        }

        return !m_Stop;
    }

    void Client::DoStop()
    {
        std::cout << "Stopping......" << std::endl;

        m_Stop = true;

        if(m_Socket.is_open())
        {
            m_Socket.close();
        }
    }

    Client::~Client()
    {
        m_Socket.close();
        std::cout << "Client closed." << std::endl;
    }

    // called from the main thread since the io service runs in its own thread, the stop is scheduled in the IO service
    void Client::Stop()
    {
        m_Ios.post(boost::bind(&Client::DoStop, this));
    }
}