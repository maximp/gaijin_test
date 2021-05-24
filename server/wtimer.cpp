#include "wtimer.hpp"
#include "server.hpp"

#include <fstream>
#include <iostream>

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
