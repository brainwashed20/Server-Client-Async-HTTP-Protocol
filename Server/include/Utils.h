#pragma once

#include "TgaHeader.h"
#include "Typedefs.h"

namespace AsyncServer
{
    class Utils
    {
    public:

        static void EncodeInt(std::vector<char>&, const unsigned int&);

        static void EncodeInt(PoolAllocCharVector&, const unsigned int&);

        static void DecodeInt(std::vector<char>&, unsigned int&);

        static void DecodeHeader(std::vector<char>&, unsigned int&);

        static void AppendString(std::vector<char>&, const std::string&);

        static void AppendString(PoolAllocCharVector&, const std::string&);

        static void GetString(std::vector<char>&, std::string&);

        static bool CompareTgaHeaders(const std::pair<TgaHeader, std::string>&, const std::pair<TgaHeader, std::string>&);

        static bool CompareOutputChunks(ReceivedChunkPtr, ReceivedChunkPtr);
    };
}
