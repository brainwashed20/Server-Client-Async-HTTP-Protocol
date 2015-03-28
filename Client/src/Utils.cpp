#include "Utils.h"

namespace AsyncClient
{
    void Utils::EncodeInt(std::vector<char>& inputData, const unsigned int& value)
    {
        inputData.push_back((value >> 24) & 0xFF);
        inputData.push_back((value >> 16) & 0xFF);
        inputData.push_back((value >> 8) & 0xFF);
        inputData.push_back(value & 0xFF);
    }

    void Utils::DecodeInt(std::vector<char>& inputData, unsigned int& myValue)
    {
        unsigned int numBytes = 4;
        unsigned int value = 0;

        if(inputData.empty())
        {
            return;
        }

        for(size_t i = 0; i < numBytes && !inputData.empty(); ++i)
        {
            value |= (int) ((unsigned char) inputData.back() << (i * 8));
            inputData.pop_back();
        }

        myValue = value;
    }

    void Utils::DecodeHeader(std::vector<char>& inputData, unsigned int& myValue)
    {
        unsigned int numBytes = 4;
        unsigned int value = 0;

        for(size_t i = 0; i < numBytes; ++i)
        {
            value |= (int) ((unsigned char) inputData[4 - i - 1]) << (i * 8);
        }

        myValue = value;
    }

    void Utils::AppendString(std::vector<char>& inputData, const std::string& myString)
    {
        std::string toAdd = "~" + myString + "~";
        std::copy(toAdd.begin(), toAdd.end(), std::back_inserter(inputData));
    }

    void Utils::GetString(std::vector<char>& inputData, std::string& myString)
    {
        std::string str;

        // removing '~'
        if(!inputData.empty() && inputData.back() == '~')
        {
            inputData.pop_back();
        }

        // removing filename characters
        while(!inputData.empty() && inputData.back() != '~')
        {
            str.push_back(inputData.back());
            inputData.pop_back();
        }

        std::reverse(str.begin(), str.end());
        myString = str;

        // removing '~'
        if(!inputData.empty() && inputData.back() == '~')
        {
            inputData.pop_back();
        }
    }

    void Utils::ConvertTgaChunkToBlackAndWhite(std::vector<char>& inputChunk, const unsigned int& inputChunkBpp)
    {
        float luma;
        unsigned int intLuma, smallChunk;

        if(inputChunkBpp == 32)
        {
            smallChunk = 4;
        }
        else
        {
            smallChunk = 3;
        }

        for(size_t i = 0; i < inputChunk.size() - smallChunk; i += smallChunk)
        {
            luma = (inputChunk[i] + inputChunk[i + 1] + inputChunk[i + 2]) / 3;

            intLuma = (unsigned int) luma;
            intLuma = std::min(intLuma, (unsigned int) 255);

            inputChunk[i] = inputChunk[i + 1] = inputChunk[i + 2] = intLuma;
            if(inputChunkBpp == 32)
            {
                inputChunk[i + 3] = 255;
            }
        }
    }
}