/**
 * @file simple_server.cpp
 * @author Ryang
 * @brief simple echo server with asio c++
 * @version 0.1
 * @date 2021-05-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <iostream>
#include <chrono>
#include <csignal>
#include <thread>

#include "server.hpp"

bool running = true;

int main(int argc, char const *argv[])
{
    signal(SIGINT, [](int signum){ std::cout << "\r[server] receive ctrl+c .." << std::endl; running = false;});

    net::Server server;
    server.run();

    while(running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "[server] shutdown .." << std::endl;
    return 0;
}
