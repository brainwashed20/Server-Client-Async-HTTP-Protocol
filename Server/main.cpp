#include "Requirements.h"

#include "Server.h"

void StartWork()
{
    boost::asio::io_service ios;

    unsigned int serverPort(60000), numOfCores(std::thread::hardware_concurrency());
    std::string inputDir, outputDir;

    AsyncServer::Server server(ios, serverPort);

    std::vector<std::shared_ptr<std::thread>> numThreads(numOfCores);

    for(size_t i = 0; i < numThreads.size(); ++i)
    {
        auto thisThread = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &ios));
        numThreads[i] = thisThread;
    }
}

int main()
{
    StartWork();

    return 0;
}
