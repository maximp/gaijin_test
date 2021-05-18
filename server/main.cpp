#include "server.hpp"

#include <iostream>
#include <string>

#include <boost/thread.hpp>

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
    srv.accept();

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
