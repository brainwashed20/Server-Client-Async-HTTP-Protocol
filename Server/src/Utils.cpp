#include "Utils.h"

namespace AsyncServer
{
    void Utils::EncodeInt(std::vector<char>& inputData, const unsigned int& value)
    {
        inputData.push_back((value >> 24) & 0xFF);
        inputData.push_back((value >> 16) & 0xFF);
        inputData.push_back((value >> 8) & 0xFF);
        inputData.push_back(value & 0xFF);
    }

    void Utils::EncodeInt(PoolAllocCharVector& inputData, const unsigned int& value)
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

        if(inputData.empty())
        {
            return;
        }

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

    void Utils::AppendString(PoolAllocCharVector& inputData, const std::string& myString)
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

    bool Utils::CompareTgaHeaders(const std::pair<TgaHeader, std::string>& left, const std::pair<TgaHeader, std::string>& right)
    {
        return left.second < right.second;
    }

    bool Utils::CompareOutputChunks(ReceivedChunkPtr leftPtr, ReceivedChunkPtr rightPtr)
    {
        if(std::get<0>(*leftPtr) == std::get<0>(*rightPtr))
        {
            return std::get<1>(*leftPtr) < std::get<1>(*rightPtr);
        }
        return std::get<0>(*leftPtr) < std::get<0>(*rightPtr);
    }
}