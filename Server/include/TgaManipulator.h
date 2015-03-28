#pragma once

#include "TgaHeader.h"
#include "Includes.h"

namespace AsyncServer
{
    namespace TgaManipulator
    {
        void LoadTGA(std::vector<char>&, const char*, TgaHeader*);
        void LoadTGA(std::vector<char>&, const std::vector<char>&, TgaHeader*);
        int WriteTGA(const char*, const std::vector<char>&, TgaHeader*);
        void LoadUncompressedImage(char*, char*, TgaHeader*);
    }
}