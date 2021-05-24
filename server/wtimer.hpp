#pragma once

#include "config.hpp"

#include <filesystem>

class server;

class write_timer : asio::steady_timer
{
public:
    write_timer(asio::io_service& ios, server& srv);
    void schedule(duration d, std::filesystem::path cfgfile);

private:
    server& _server;

    void write(const std::filesystem::path& cfgfile);
};
