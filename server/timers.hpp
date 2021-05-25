#pragma once

#include "config.hpp"

#include <filesystem>

class server;

class base_server_timer : asio::steady_timer
{
public:
    base_server_timer(asio::io_service& ios, server& srv);
    void schedule(duration d);

protected:
    server& _server;

    virtual void expired() = 0;
};

class write_timer : public base_server_timer
{
public:
    write_timer(asio::io_service& ios, server& srv, std::filesystem::path cfgfile);

private:
    const std::filesystem::path _cfgfile;

    virtual void expired() override;
};

class stat_timer : public base_server_timer
{
public:
    stat_timer(asio::io_service& ios, server& srv);

private:
    virtual void expired() override;
};
