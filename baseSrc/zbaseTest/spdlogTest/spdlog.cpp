
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "log/Logging.h"
using namespace std;


void daily_example()
{
    // Create a daily logger - a new file is created every day on 2:30am.
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
    for(int i = 0; i < 100; ++i){
        daily_logger->info("this is daily logger mt....{}", i);
    }
}


#include "spdlog/async.h"
void async_example()
{
    // Default thread pool settings can be modified *before* creating the async logger:
    // spdlog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
    // alternatively:
    // auto async_file = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("async_file_logger", "logs/async_log.txt");

    for (int i = 1; i < 101; ++i)
    {
        async_file->info("Async message #{}", i);
    }
}

#include "spdlog/fmt/bin_to_hex.h"
void binary_example()
{
    std::vector<char> buf(80);
    for (int i = 0; i < 80; i++)
    {
        buf.push_back(static_cast<char>(i & 0xff));
    }

    spdlog::info("Binary example: {}", spdlog::to_hex(buf));
    spdlog::info("Another binary example:{:n}", spdlog::to_hex(std::begin(buf), std::begin(buf) + 10));
    // more examples:
    spdlog::info("uppercase: {:X}", spdlog::to_hex(buf));
    spdlog::info("uppercase, no delimiters: {:Xs}", spdlog::to_hex(buf));
    spdlog::info("uppercase, no delimiters, no position info: {:Xsp}", spdlog::to_hex(buf));
    spdlog::info("hexdump style: {:a}", spdlog::to_hex(buf));
    spdlog::info("hexdump style, 20 chars per line {:a}", spdlog::to_hex(buf, 20));
}


// Log a vector of numbers
#ifndef SPDLOG_USE_STD_FORMAT
#    include "spdlog/fmt/ranges.h"
void vector_example()
{
    std::vector<int> vec = {1, 2, 3, 4, 5, 6};
    spdlog::info("Vector example: {}", vec);
}

#else
void vector_example() {}
#endif

int main(int argc, char* argv[]){
    LOG_RED << "HELLO";
    return 0;
}
