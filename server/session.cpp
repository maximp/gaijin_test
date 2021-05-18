#include "session.hpp"

#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#define TRACE 1

session::session(asio::ip::tcp::socket s)
: _s(std::move(s)), _ep(_s.remote_endpoint())
{
#ifdef TRACE
    std::cout << "incoming connection from: " << _ep << std::endl;
#endif
    _handlers.emplace("quit", command{[this](){ handle_quit(); }, ""});
    _handlers.emplace("get", command{[this](){ handle_get(); }, "get <key-name>"});
    _handlers.emplace("set", command{[this](){ handle_set(); }, "set <key-name>=<key-value>"});
}

session::~session()
{
#ifdef TRACE
    std::cout << "connection closed: " << _ep << std::endl;
#endif
}

void session::start()
{
    async_read();
}

void session::async_read()
{
    asio::async_read_until(_s, asio::dynamic_buffer(_msg, 1024), "\r\n",
        [self = shared_from_this(), this] (boost::system::error_code ec, std::size_t bytes_transferred)
        {
            if(ec)
            {
#ifdef TRACE
                std::cerr << _ep << " read error: " << ec << std::endl;
#endif
                return;
            }

            handle_command();
            async_read();
        });
}

void session::handle_command()
{
    boost::trim(_msg);

#ifdef TRACE
    std::cout << _ep << ": " << _msg << std::endl;
#endif

    _parts.clear();

    // assume, key name/value does not contain spaces
    // in real life helper class command_parser may be implemented to parse command line
    boost::split(_parts, _msg, boost::is_any_of(" "));
    _msg.clear();

    if(_parts.empty())
    {
        write("invalid command\n");
        return;
    }

    auto it = _handlers.find(_parts[0]);
    if(it == _handlers.end())
    {
        write("command not found\n");
        return;
    }

    const command& cmd = it->second;

    try
    {
        return cmd._handler();
    }
    catch(const std::exception& e)
    {
        std::ostringstream os;
        os << "invalid '" << _parts[0] << "' command, usage: " << cmd._usage;
        write(os.str());
    }
}

void session::handle_quit()
{
    write("bye\n");
}

void session::handle_get()
{
    if(_parts.size() != 2)
    {
        write("invalid 'get' command, usage: get <key-name>");
        return;
    }
}

void session::handle_set()
{

    if(_parts.size() != 2)
        throw std::length_error("");

    std::vector<std::string> data;
    boost::split(data, _parts[1], boost::is_any_of("="));

    if(data.empty())
        throw std::invalid_argument("");

    std::string value;
    if(data.size() >= 2)
        value = std::move(data[1]);
 }

void session::write(std::string s)
{
    std::shared_ptr<const std::string> ps = std::make_shared<std::string>(std::move(s));
    asio::async_write(_s, asio::buffer(*ps),
        [ps, self = shared_from_this(), this](boost::system::error_code ec, std::size_t bytes_transferred) {
            if(!ec)
                return;
#ifdef TRACE
            std::cerr << _ep << " write error: " << ec << std::endl;
#endif
            _s.close();
        });
}
