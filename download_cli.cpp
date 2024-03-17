#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <ranges>
#include <range/v3/all.hpp>
#include <thread>
#include <tuple>

#include <boost/program_options.hpp>
#include <fmt/core.h>


#include "utils/date_range.hpp"

namespace po = boost::program_options;
using namespace std::chrono;

int main(int argc, char* argv[]) {
    std::string symbol, start_str, end_str;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("symbol", po::value<std::string>(&symbol)->required(), "set symbol")
        ("start", po::value<std::string>(&start_str)->required(), "set start date")
        ("end", po::value<std::string>(&end_str)->required(), "set end date");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    year_month_day start = string_to_sys_days(start_str);
    year_month_day end = string_to_sys_days(end_str);

    std::cout << "Symbol: " << symbol << "\n";
    std::cout << "Start date: " << start << "\n";
    std::cout << "End date: " << end << "\n";
        
    return 0;
}