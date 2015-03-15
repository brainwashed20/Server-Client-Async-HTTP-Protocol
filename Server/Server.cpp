#include "Requirements.h"

int main(int argc, char* argv[])
{
    asio::io_service io_service;
    asio::io_service::work work(io_service);

    std::vector<cpp11::thread> threadPool;

    for(size_t t = 0; t < cpp11::thread::hardware_concurrency(); t++)
    {
        threadPool.push_back(cpp11::thread(boost::bind(&asio::io_service::run, &io_service)));
    }

    //Do some things with the main thread

    io_service.stop();
    for(cpp11::thread& t : threadPool)
    {
        t.join();
    }
}