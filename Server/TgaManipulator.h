#pragma once

#include "Requirements.h"

namespace AsyncServer
{
        #pragma pack(push, x1)					// Byte alignment (8-bit)
        #pragma pack(1)

    struct TgaHeader
    {
        unsigned char  identsize;			// size of ID field that follows 18 byte header (0 usually)
        unsigned char  colourmaptype;		// type of colour map 0=none, 1=has palette
        unsigned char  imagetype;			// type of image 2=rgb uncompressed, 10 - rgb rle compressed

        short colourmapstart;				// first colour map entry in palette
        short colourmaplength;				// number of colours in palette
        unsigned char  colourmapbits;		// number of bits per palette entry 15,16,24,32

        short xstart;						// image x origin
        short ystart;						// image y origin
        short width;						// image width in pixels
        short height;						// image height in pixels
        unsigned char  bits;				// image bits per pixel 24,32
        unsigned char  descriptor;			// image descriptor bits (vh flip bits)

        // pixel data follows header
    };

    #pragma pack(pop, x1)

    namespace TgaManipulator
    {
        void LoadTGA(std::vector<char>&, const char*, TgaHeader*);
        void LoadTGA(std::vector<char>&, const std::vector<char>&, TgaHeader*);
        int WriteTGA(const char*, const std::vector<char>&, TgaHeader*);
        void LoadUncompressedImage(char*, char*, TgaHeader*);
    }

    namespace TgaManipulator
    {
        const int k_ItCompressed = 10;
        const int k_ItUncompressed = 2;

        void LoadCompressedImage(char *outputData, char *inputData, TgaHeader *myHeader)
        {
            int imageWidth = myHeader->width;
            int imageHeigth = myHeader->height;

            int rowSize = imageWidth * myHeader->bits / 8;

            int countPixels = 0;
            int pixelsNumber = imageWidth * imageHeigth;

            bool isInverted = ((myHeader->descriptor & (1 << 5)) != 0);

            char *outputPtrData = isInverted ? outputData + (imageHeigth + 1) * rowSize : outputData;

            while(pixelsNumber > countPixels)
            {
                unsigned char chunk = *inputData++;
                if(chunk < 128)
                {
                    int chunkSize = chunk + 1;
                    for(int i = 0; i < chunkSize; i++)
                    {
                        if(isInverted && (countPixels % imageWidth) == 0)
                        {
                            outputPtrData -= 2 * rowSize;
                        }

                        *outputPtrData++ = inputData[2];
                        *outputPtrData++ = inputData[1];
                        *outputPtrData++ = inputData[0];

                        inputData += 3;

                        if(myHeader->bits != 24)
                        {
                            *outputPtrData++ = *inputData++;
                        }

                        countPixels++;
                    }
                }
                else
                {
                    int chunkSize = chunk - 127;
                    for(int i = 0; i < chunkSize; i++)
                    {
                        if(isInverted && (countPixels % imageWidth) == 0)
                        {
                            outputPtrData -= 2 * rowSize;
                        }

                        *outputPtrData++ = inputData[2];
                        *outputPtrData++ = inputData[1];
                        *outputPtrData++ = inputData[0];

                        if(myHeader->bits != 24)
                        {
                            *outputPtrData++ = inputData[3];
                        }

                        countPixels++;
                    }

                    inputData += (myHeader->bits >> 3);
                }
            }
        }

        void LoadUncompressedImage(char* outputData, char* inputData, TgaHeader* myHeader)
        {
            int imageWidth = myHeader->width;
            int imageHeigth = myHeader->height;
            int rowSize = imageWidth * myHeader->bits / 8;

            bool isInverted = ((myHeader->descriptor & (1 << 5)) != 0);

            for(int i = 0; i < imageHeigth; i++)
            {
                char* inputPtrRow = inputData + (isInverted ? (imageHeigth - i - 1) * rowSize : i * rowSize);
                if(myHeader->bits == 24)
                {
                    for(int j = 0; j < imageWidth; j++)
                    {
                        *outputData++ = inputPtrRow[2];
                        *outputData++ = inputPtrRow[1];
                        *outputData++ = inputPtrRow[0];
                        inputPtrRow += 3;
                    }
                }
                else
                {
                    for(int j = 0; j < imageWidth; j++)
                    {
                        *outputData++ = inputPtrRow[2];
                        *outputData++ = inputPtrRow[1];
                        *outputData++ = inputPtrRow[0];
                        *outputData++ = inputPtrRow[3];
                        inputPtrRow += 4;
                    }
                }
            }
        }

        void LoadTGA(std::vector<char>& convertedImageData, const std::vector<char>& imageData, TgaHeader* myHeader)
        {
            char *outputCharData = new char[myHeader->width * myHeader->height * myHeader->bits / 8];
            char *inputCharData = new char[myHeader->width * myHeader->height * myHeader->bits / 8];

            std::copy(imageData.begin(), imageData.end(), inputCharData);

            switch(myHeader->imagetype)
            {
                case k_ItUncompressed:
                    LoadUncompressedImage(outputCharData, inputCharData, myHeader);
                    break;
                case k_ItCompressed:
                    LoadCompressedImage(outputCharData, inputCharData, myHeader);
                    break;
            }

            std::copy(outputCharData, outputCharData + myHeader->width * myHeader->height * myHeader->bits / 8, std::back_inserter(convertedImageData));

            // delete [] outputCharData;
            // delete [] inputCharData;
        }
        void LoadTGA(std::vector<char>& imageData, const char* inputFileName, TgaHeader* myHeader)
        {
            FILE* f;

            if(fopen_s(&f, inputFileName, "rb") != 0)
            {
                return;
            }

            TgaHeader header;
            fread(&header, sizeof(header), 1, f);

            fseek(f, 0, SEEK_END);
            int inputFileLength = ftell(f);
            fseek(f, sizeof(header) +header.identsize, SEEK_SET);

            if(header.imagetype != k_ItCompressed && header.imagetype != k_ItUncompressed)
            {
                fclose(f);
                return;
            }

            if(header.bits != 24 && header.bits != 32)
            {
                fclose(f);
                return;
            }

            int bufferSize = inputFileLength - sizeof(header) -header.identsize;
            char* inputCharData = new char[bufferSize];
            fread(inputCharData, 1, bufferSize, f);
            fclose(f);

            char* outputCharData = new char[header.width * header.height * header.bits / 8];

            switch(header.imagetype)
            {
                case k_ItUncompressed:
                    LoadUncompressedImage(outputCharData, inputCharData, &header);
                    break;
                case k_ItCompressed:
                    LoadCompressedImage(outputCharData, inputCharData, &header);
                    break;
            }

            std::copy(outputCharData, outputCharData + header.width * header.height * header.bits / 8, std::back_inserter(imageData));
            memcpy(myHeader, &header, sizeof(header));

            delete[] inputCharData;
            delete[] outputCharData;
        }

        int WriteTGA(const char* inputFileName, const std::vector<char>& imageData, TgaHeader* myHeader)
        {
            FILE* f;

            if(fopen_s(&f, inputFileName, "wb") != 0)
            {
                return -1;
            }

            fwrite(myHeader, sizeof(*myHeader), 1, f);

            char *imageDataBuffer = new char[imageData.size()];
            std::copy(imageData.begin(), imageData.end(), imageDataBuffer);

            fwrite(imageDataBuffer, 1, myHeader->width * myHeader->height * myHeader->bits / 8, f);

            delete[] imageDataBuffer;

            fclose(f);

            return 1;
        }
    }
}