#pragma once

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <ranges>

namespace duco::data {

    struct File
    {
    public:
        static std::string getFilepath(std::string symbol, std::chrono::year_month_day ymd)
        {
            auto const y = static_cast<int>(ymd.year());
            auto const m = static_cast<unsigned>(ymd.month());
            auto const d = static_cast<unsigned>(ymd.day());
            return std::format("./data/{0}/{1}-{2:02d}/{0}_{1}-{2:02d}-{3:02d}.zst", symbol, y, m, d);
        }
    };

    struct RefData
    {
    public:
        static const int get(const std::string& str) { return Sym2ID.at(str); }
    private:
        static const inline std::map<std::string, int> Sym2ID = {
            {"EURUSD", 1},
            {"GBPUSD", 2},
        };
    };

    struct TickRow
    {
        uint32_t timestamp;
        uint32_t askPrice;
        uint32_t bidPrice;
        float askVolume;
        float bidVolume;

        uint32_t symbol;

        TickRow(){}

        // TickRow(std::span<uint32_t, 5> data)
        //     : timestamp(data[0]),
        //     askPrice(data[1]),
        //     bidPrice(data[2]),
        //     askVolume(data[3]),
        //     bidVolume(data[4])
        // {
        //     // Additional initialization if needed
        // }

        template <typename Range>
        TickRow(const Range& range) { initializeFromRange(range); }

        TickRow(int sym, auto const& range)
        {
            symbol = sym;
            initializeFromRange(range);
        }

        // Overload << for ostream to print TickRow
        friend std::ostream& operator<<(std::ostream& os, const TickRow& tickRow)
        {
            os << "(" 
            << tickRow.symbol << ", "
            << tickRow.timestamp << ", "
            << tickRow.askPrice << ", "
            << tickRow.bidPrice << ", "
            << std::fixed << std::setprecision(3) << tickRow.askVolume << ", "
            << std::fixed << std::setprecision(3) << tickRow.bidVolume << ")";
            return os;
        }

        // TODO: Equals should check every field
        bool operator==(const TickRow& other) const { return timestamp == other.timestamp; }
        bool operator!=(const TickRow& other) const { return timestamp != other.timestamp; }

        // Comparison operators based on timestamp
        bool operator<(const TickRow& other) const { return timestamp < other.timestamp; }
        bool operator>(const TickRow& other) const { return timestamp > other.timestamp; }
        bool operator<=(const TickRow& other) const { return timestamp <= other.timestamp; }
        bool operator>=(const TickRow& other) const { return timestamp >= other.timestamp; }

        void initializeFromRange(auto const& range)
        {
            auto it = std::ranges::begin(range);
            auto end = std::ranges::end(range);

            if (it != end) { timestamp = *it; it++; }
            if (it != end) { askPrice = *it; it++; }
            if (it != end) { bidPrice = *it; it++; }

            // TODO: Volumes need to be multiplied by instrument specific multipliers eg. 1e6
            if (it != end) { askVolume = std::bit_cast<float>(*it); it++; }
            if (it != end) { bidVolume = std::bit_cast<float>(*it); it++; }
        }

    };
}