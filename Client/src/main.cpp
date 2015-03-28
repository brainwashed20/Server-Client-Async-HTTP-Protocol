#include "Includes.h"
#include "Client.h"

void MakeConfig(std::string& _serverHost, unsigned int& _serverPort)
{
    _serverHost = "localhost";
    _serverPort = 60000;
}

static void StartWork()
{
    std::string serverHost;
    unsigned int serverPort;

    MakeConfig(serverHost, serverPort);

    boost::asio::io_service asioIoService;

    boost::asio::ip::tcp::resolver asioTcpResolver(asioIoService);

    boost::asio::ip::tcp::resolver::query asioTcpResolverQuery(
        boost::asio::ip::tcp::v4(),
        serverHost,
        std::to_string(serverPort)
    );

    boost::asio::ip::tcp::resolver::iterator asioTcpResolverIterator = asioTcpResolver.resolve(asioTcpResolverQuery);

    AsyncClient::Client tgaClient(asioIoService);
    tgaClient.Start(asioTcpResolverIterator);

    std::thread ioServiceThread(boost::bind(&boost::asio::io_service::run, &asioIoService));

    int value;
    std::cin >> value;

    std::cout << "Client stopping asynchronously..." << std::endl;

    tgaClient.Stop();

    ioServiceThread.join();
}

int main() 
{
	StartWork();

	return 0;
}