#pragma once

#include <chrono>
#include <span>
#include <string>
#include <vector>

// #include "__generator.hpp"
#include "utils.hpp"

namespace duco
{
    using namespace std::chrono;

    struct Response
    {
        std::string mdata;
        std::string headers;
        long status;

        template<typename Range = std::span<uint8_t>, typename T = typename Range::value_type>
        Range data()
        { 
            // return make_range<Range>(reinterpret_cast<T*>(mdata.data()), mdata.size());
            return RangeMaker<Range, T>::make(reinterpret_cast<T*>(mdata.data()), mdata.size());
        }

        // This will not work for vector
        // template<typename Range = std::span<uint8_t>, typename T = typename Range::value_type>
        // Range data()
        // { 
        //     return Range(reinterpret_cast<T*>(mdata.data()), mdata.size());
        // }

        std::string text() { return mdata; }
    };

    class Downloader
    {
    public:
        static std::string getFilepath(std::string symbol, year_month_day ymd);
        static void downloadFile(std::string symbol, year_month_day ymd);
        static std::vector<uint8_t> loadData(std::string symbol, year_month_day ymd);

    private:
        static Response const fetch(std::string const& url);
        static size_t ResponseCallback(void* contents, size_t size, size_t nmemb, void* userp);

        static Generator<uint8_t> streamBytes(std::string symbol, year_month_day ymd);

        // static std::generator<uint8_t> streamBytes();
        // static Generator<uint32_t> stream();
        // static Generator<std::array<uint32_t, 5>> streamRows();
    };

}