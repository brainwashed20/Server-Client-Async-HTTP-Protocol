#pragma once

#include "Includes.h"

namespace AsyncServer
{
    typedef std::queue<std::shared_ptr<std::vector<char> > > InitialChunks;
    typedef std::shared_ptr<std::vector<char> > InitialChunkPtr;

    typedef std::deque<std::shared_ptr<std::tuple<std::string, unsigned int, std::vector<char> > > > ReceivedChunks;
    typedef std::shared_ptr<std::tuple<std::string, unsigned int, std::vector<char> > > ReceivedChunkPtr;
    typedef std::tuple<std::string, unsigned int, std::vector<char> > ReceivedChunk;

    typedef std::vector<char, boost::pool_allocator<char> > PoolAllocCharVector;
}

