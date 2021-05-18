#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

using namespace boost;

int main(int argc, char** argv)
{
    unsigned short port = 12345;
    if(argc >= 2)
    {
        try
        {
            port = std::stoul(argv[1]);
        }
        catch(const std::exception&)
        {
            std::cerr << "Invalid server port: " << argv[1] << std::endl;
        }
    }
    else
    {
        std::cerr << "Usage: server <port>" << std::endl;
        std::cerr << "Using default port: " << port << std::endl;
    }

    asio::io_service ios;

    asio::signal_set signals(ios, SIGINT, SIGTERM);
    signals.async_wait([&ios](auto, auto){ ios.stop(); });

    return 0;
}
