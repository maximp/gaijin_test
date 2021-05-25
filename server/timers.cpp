#include "timers.hpp"
#include "server.hpp"

#include <fstream>
#include <iostream>

base_server_timer::base_server_timer(asio::io_service& ios, server& srv)
:   asio::steady_timer(ios),
    _server(srv)
{}

void base_server_timer::schedule(duration d)
{
    expires_from_now(d);
    async_wait([this, d](boost::system::error_code ec) {
        if(ec)
            return;
        expired();
        schedule(d);
    });
}

write_timer::write_timer(asio::io_service& ios, server& srv, std::filesystem::path cfgfile)
:   base_server_timer(ios, srv),
    _cfgfile(std::move(cfgfile))
{}

void write_timer::expired()
{
    try
    {
        if(!_server.modified())
            return;

        std::ofstream os;
        os.exceptions(std::ifstream::failbit);
        os.open("." / _cfgfile, std::ifstream::out);
        os.exceptions(std::ifstream::goodbit);

        _server.write(os);

        std::cout << "Saved " << _cfgfile << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed writing config file " << _cfgfile << ": " << e.what() << std::endl;
    }
}

stat_timer::stat_timer(asio::io_service& ios, server& srv)
:   base_server_timer(ios, srv)
{}

void stat_timer::expired()
{
    try
    {
        server::stats_t s = _server.stats();
        std::cout << s << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed query server stat: " << e.what() << std::endl;
    }
}
