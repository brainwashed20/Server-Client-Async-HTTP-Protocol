#pragma once

#include "Includes.h"

namespace AsyncClient
{
    class Utils
    {
    public:

        static void ConvertTgaChunkToBlackAndWhite(std::vector<char>&, const unsigned int&);

        static void EncodeInt(std::vector<char>&, const unsigned int&);

        static void DecodeInt(std::vector<char>&, unsigned int&);

        static void DecodeHeader(std::vector<char>&, unsigned int&);

        static void AppendString(std::vector<char>&, const std::string&);

        static void Utils::GetString(std::vector<char>&, std::string&);
    };
}
