#pragma once

#include <coroutine>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <fstream>
#include <iostream>
#include <span>
#include <stdint.h>
#include <type_traits>

#include <lzma.h>

#include "concepts.hpp"

namespace duco
{

/*
std::vector<uint8_t> DecompressStream(std::ifstream& inputFile)
{
    std::vector<uint8_t> decompressedData;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK) {
        std::cerr << "LZMA initialization failed" << std::endl;
        return decompressedData;
    }

    std::vector<uint8_t> inBuffer(4096); // Input buffer
    std::vector<uint8_t> outBuffer(4096); // Output buffer

    while (true)
    {
        inputFile.read(reinterpret_cast<char*>(inBuffer.data()), inBuffer.size());
        strm.avail_in = inputFile.gcount(); // Number of bytes read
        if (strm.avail_in == 0) break; // End of input

        strm.next_in = inBuffer.data();
        while (strm.avail_in > 0) {
            strm.avail_out = outBuffer.size();
            strm.next_out = outBuffer.data();

            ret = lzma_code(&strm, LZMA_RUN);
            if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
                std::cerr << "LZMA decompression failed" << std::endl;
                return decompressedData;
            }

            size_t have = outBuffer.size() - strm.avail_out;
            decompressedData.insert(decompressedData.end(), outBuffer.begin(), outBuffer.begin() + have);
        }
    }

    lzma_end(&strm);
    
    return decompressedData;
}


std::vector<uint8_t> DecompressFile(std::string filepath)
{
    std::ifstream ifs(filepath, std::ios::binary);
    return DecompressStream(ifs);
}
*/


template <ByteType B>
std::vector<B> DecompressLzmaBytes(std::span<B> inputSpan)
{
    std::vector<B> decompressedData;

    if (inputSpan.empty()) return decompressedData;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK) {
        std::cerr << "LZMA initialization failed" << std::endl;
        return decompressedData;
    }

    std::vector<B> outBuffer(4096);

    while (!inputSpan.empty())
    {
        strm.avail_in = inputSpan.size();
        strm.next_in = reinterpret_cast<uint8_t*>(inputSpan.data());
        
        while (strm.avail_in > 0)
        {
            strm.avail_out = outBuffer.size();
            strm.next_out = reinterpret_cast<uint8_t*>(outBuffer.data());

            ret = lzma_code(&strm, LZMA_RUN);
            if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
                std::cerr << "LZMA decompression failed" << std::endl;
                return decompressedData;
            }

            size_t have = outBuffer.size() - strm.avail_out;
            decompressedData.insert(decompressedData.end(), outBuffer.begin(), outBuffer.begin() + have);
        }
        inputSpan = inputSpan.subspan(inputSpan.size() - strm.avail_in);
    }

    lzma_end(&strm);

    return decompressedData;
}

// Explicit instantiation of template is required if using a separated header and source file :-(
// template std::vector<uint8_t> DecompressBytes(std::span<uint8_t> input_data);


}

// DecompressLzma("test.cpp.lzma", "output.txt");