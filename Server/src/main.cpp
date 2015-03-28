#include "Includes.h"
#include "Server.h"

void MakeConfig(unsigned int& _serverPort, std::string& _inputDir, std::string& _outputDir, unsigned int& _numOfCores)
{
    _serverPort = 60000;
    _inputDir = "initialTGA";
    _outputDir = "convertedTGA";
    _numOfCores = std::thread::hardware_concurrency();
}

void StartWork()
{
    boost::asio::io_service ios;

    unsigned int serverPort(0), numOfCores(0);
    std::string inputDir, outputDir;

    MakeConfig(serverPort, inputDir, outputDir, numOfCores);

    AsyncServer::Server server(ios, serverPort);

    std::vector<std::shared_ptr<std::thread>> numThreads(numOfCores);

    for(size_t i = 0; i < numThreads.size(); ++i)
    {
        auto thisThread = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &ios));
        numThreads[i] = thisThread;
    }

    std::thread tgaProcessingThread(&AsyncServer::Server::ProcessInputTgaFolder, std::ref(server), inputDir, outputDir);

    for(size_t i = 0; i < numThreads.size(); ++i)
    {
        numThreads[i]->join();
    }

    tgaProcessingThread.join();
}

int main()
{
    StartWork();

    return 0;
}
