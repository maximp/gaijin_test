#include "server.hpp"
#include "wtimer.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/thread.hpp>

static const std::filesystem::path CONFIG_FILE = "config.txt";
static const int WRITE_INTERVAL = 5; // seconds

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

    server srv(ios, port);

    try
    {
        std::cout << "Current: " << std::filesystem::current_path() << std::endl;

        std::ifstream is;
        is.exceptions(std::ifstream::failbit);
        is.open("." / CONFIG_FILE, std::ifstream::in);
        is.exceptions(std::ifstream::goodbit);
        srv.read(is);

        std::cout << "Loaded " << CONFIG_FILE << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed reading config file " << CONFIG_FILE << ": " << e.what() << std::endl;
    }

    srv.start();

    write_timer wt(ios, srv);
    wt.schedule(std::chrono::seconds(WRITE_INTERVAL), CONFIG_FILE);

    boost::thread_group tg;
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i)
        tg.create_thread([&ios]()
        {
            while(!ios.stopped())
            try
            {
                ios.run();
            }
            catch(const std::exception& e)
            {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
        });
    tg.join_all();
}
