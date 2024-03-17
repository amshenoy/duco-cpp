#include <array>
#include <chrono>
#include <coroutine>
#include <format>
#include <future>
#include <ios>
#include <iostream>
#include <map>
#include <numeric>
#include <stdint.h>
#include <string>
#include <ranges>
#include <thread>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zstd.hpp>
#include <curl/curl.h>

#include "decompress.hpp"
#include "downloader.hpp"
#include "concepts.hpp"

using namespace std::chrono_literals;

namespace duco
{
namespace bio = boost::iostreams;

Response const Downloader::fetch(std::string const& url)
{
    CURL* curl = curl_easy_init();
    Response responseData;

    if (!curl)
    {
        std::cerr << "Failed to load CURL" << std::endl;
        return responseData;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData.mdata);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseData.headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "Failed to fetch URL: " << url << " (" << curl_easy_strerror(res) << ")\n";
    }
    else
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseData.status);
    }

    curl_easy_cleanup(curl);

    return responseData;
}
        
size_t Downloader::ResponseCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// template<ByteType B>
Generator<uint8_t> Downloader::streamBytes(std::string symbol, std::chrono::year_month_day ymd)
{
    auto downloadLambda = [&symbol, &ymd](int hour){
        auto constexpr urlFormat = "https://www.dukascopy.com/datafeed/{0}/{1}/{2:02}/{3:02}/{4:02}h_ticks.bi5";
        int const y = static_cast<int>(ymd.year()), m = static_cast<unsigned>(ymd.month()) - 1, d = static_cast<unsigned>(ymd.day());
        auto const url = std::format(urlFormat, symbol, y, m, d, hour);
        Response fetched;
        int backoff = 1;
        int backoffStep = 3;
        int backoffPeriod = 100;
        while (fetched.status != 200 and fetched.status != 404) 
        {
            std::this_thread::sleep_for(milliseconds(backoffPeriod*backoff));
            fetched = Downloader::fetch(url);
            backoff += backoffStep;
        }
        if (fetched.status == 404) return Response{};
        return fetched;
    };

    auto hours = std::views::iota(0, 24);
    std::map<int, std::future<Response>> futures;

    for (auto hour : hours)
    {
        futures[hour] = std::async(std::launch::async, downloadLambda, hour);
    }

    for (auto hour : hours)
    {
        auto fetched = futures[hour].get();

        // TODO: Multiplex the fetches instead of yielding immediately
        // if (fetched.status != 200)
        // {
        //     futures[hour] = std::async(std::launch::async, downloadLambda, hour);
        //     continue;
        // }

        auto res = fetched.data();
        auto b = DecompressLzmaBytes(res);

        std::cout << hour << " hour: " << b.size() << " decompressed bytes ";
        std::cout << "- Status: " << fetched.status << std::endl;

        std::array<uint32_t, 5> buf;
        auto rows = b 
        | std::views::chunk(20) 
        | std::views::transform([&buf, &hour](auto const& c20){
            std::memcpy(buf.data(), c20.data(), sizeof(buf));
            for (auto& e : buf) e = boost::endian::big_to_native(e);
            buf[0] += hour*3600*1000;
            return buf;
        });

        auto bytes4 = rows | std::views::join;
        for (auto const b4 : bytes4)
        {
            // co_yield b4; // uint32_t

            // Save as little-endian
            co_yield static_cast<uint8_t>(b4 & 0xff);
            co_yield static_cast<uint8_t>((b4 >> 8) & 0xff);
            co_yield static_cast<uint8_t>((b4 >> 16) & 0xff);
            co_yield static_cast<uint8_t>((b4 >> 24) & 0xff);

            // // Save as big-endian
            // co_yield static_cast<uint8_t>((b4 >> 24) & 0xff);
            // co_yield static_cast<uint8_t>((b4 >> 16) & 0xff);
            // co_yield static_cast<uint8_t>((b4 >> 8) & 0xff);
            // co_yield static_cast<uint8_t>(b4 & 0xff);
        }
    }
}

std::string Downloader::getFilepath(std::string symbol, std::chrono::year_month_day ymd)
{
    auto const y = static_cast<int>(ymd.year());
    auto const m = static_cast<unsigned>(ymd.month());
    auto const d = static_cast<unsigned>(ymd.day());
    return std::format("./data/{0}/{1}-{2:02d}/{0}_{1}-{2:02d}-{3:02d}.zst", symbol, y, m, d);
}

void Downloader::downloadFile(std::string symbol, std::chrono::year_month_day ymd)
{
    auto const outFile = getFilepath(symbol, ymd);
    std::cout << "Output File: " << outFile << std::endl;

    boost::filesystem::path directory = boost::filesystem::path(outFile).parent_path();
    if (!boost::filesystem::exists(directory)) boost::filesystem::create_directories(directory);

    bio::file_sink compressedFile(outFile, std::ios_base::binary);
    bio::filtering_ostream fos;
    fos.push(bio::zstd_compressor());
    fos.push(compressedFile);

    for (uint8_t byte : streamBytes(symbol, ymd)) {
        fos.put(static_cast<char>(byte));
    }
}


// This loads the entire file into memory so not the best approach
std::vector<uint8_t> Downloader::loadData(std::string symbol, std::chrono::year_month_day ymd)
{
    auto const inFile = getFilepath(symbol, ymd);
    bio::file_source compressedFile(inFile, std::ios_base::binary);
    if (!compressedFile.is_open()) throw std::runtime_error("Error opening compressed file.");

    bio::filtering_istream fis;
    fis.push(bio::zstd_decompressor());
    fis.push(compressedFile);

    // TODO: Return the stream (either as a stream, generator or iterator) so we can iterate through the bytes 
    // and if needed have an implicit conversion to a vector
    std::vector<uint8_t> decompressedData;
    try {
        bio::copy(fis, std::back_inserter(decompressedData));
    } catch (const std::exception& e) {
        std::cerr << "Error during decompression: " << e.what() << std::endl;
    }

    return decompressedData;
}







// template<ByteType B>
/*
Generator<std::array<uint32_t, 5>> Downloader::streamRows()
{
    auto const symbol = "EURUSD";
    auto year = 2020, month = 1, day = 6;
    month -= 1;

    auto downloadLambda = [&symbol, &year, &month, &day](int hour){
        auto constexpr urlFormat = "https://www.dukascopy.com/datafeed/{0}/{1}/{2:02}/{3:02}/{4:02}h_ticks.bi5";
        auto const url = std::format(urlFormat, symbol, year, month, day, hour);
        Response fetched = Downloader::fetch(url);
        auto res = fetched.data<std::vector<uint8_t>>();
        return res;
    };

    int start = 0, end = 24;
    std::map<int, std::future<std::vector<uint8_t>>> futures;

    for (int i = start; i < end; i++)
    {
        futures[i] = std::async(std::launch::async, downloadLambda, i);
    }

    std::vector<uint8_t> result;
    for (int i = start; i < end; i++)
    {
        auto hour = i;
        auto res = futures[hour].get();
        auto b = DecompressBytes(std::span{res});

        std::cout << b.size() << " decompressed bytes" << std::endl;

        std::array<uint32_t, 5> buf;
        auto rows = b 
        | std::views::chunk(20) 
        | std::views::transform([&buf, &hour](auto const& c20){
            std::memcpy(buf.data(), c20.data(), sizeof(buf));
            for (auto& e : buf) e = boost::endian::big_to_native(e);
            buf[0] += hour*3600*1000;
            return buf;
        });

        for (auto const& row : rows) {
            co_yield std::move(row);
        }
    }
}
*/




}