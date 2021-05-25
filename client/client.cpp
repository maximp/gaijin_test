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
    std::string server = "`127.0.0.1";
    unsigned short port = 12345;
    if(argc >= 2)
    {
        server = argv[1];

        if(argc >= 3)
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
    }
    else
    {
        std::cerr << "Usage: test-client <server> <port>" << std::endl;
    }

    std::cout << "Connecting " << server << ":" << port << "..." << std::endl;

    asio::io_service ios;

    asio::signal_set signals(ios, SIGINT, SIGTERM);
    signals.async_wait([&ios](auto, auto){ ios.stop(); });

    return 0;
}
