#include "server.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/thread.hpp>

static const std::filesystem::path CONFIG_FILE = "config.txt";
static const int WRITE_INTERVAL = 5; // seconds

class write_timer : asio::steady_timer
{
public:
    write_timer(asio::io_service& ios, server& srv);
    void schedule(duration d, std::filesystem::path cfgfile);

private:
    server& _server;

    void write(const std::filesystem::path& cfgfile);
};

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

write_timer::write_timer(asio::io_service& ios, server& srv)
:   asio::steady_timer(ios),
    _server(srv)
{}

void write_timer::schedule(duration d, std::filesystem::path cfgfile)
{
    expires_from_now(d);
    async_wait([this, d, cfgfile](boost::system::error_code ec) {
        if(ec)
            return;
        write(cfgfile);
        schedule(d, cfgfile);
    });
}

void write_timer::write(const std::filesystem::path& cfgfile)
{
    try
    {
        if(!_server.modified())
            return;

        std::ofstream os;
        os.exceptions(std::ifstream::failbit);
        os.open("." / cfgfile, std::ifstream::out);
        os.exceptions(std::ifstream::goodbit);

        _server.write(os);

        std::cout << "Saved " << cfgfile << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed writing config file " << cfgfile << ": " << e.what() << std::endl;
    }
}