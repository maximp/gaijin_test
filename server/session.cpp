#include "session.hpp"
#include "server.hpp"

#include <iostream>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#define TRACE 1

session::session(server& srv, asio::ip::tcp::socket s)
: _server(srv), _s(std::move(s)), _ep(_s.remote_endpoint())
{
#ifdef TRACE
    std::cout << "incoming connection from: " << _ep << std::endl;
#endif

    auto register_handler = [this](const char* name, handler_t h, std::string usage) {
        command cmd;
        cmd._handler = h;
        cmd._usage = std::move(usage);
        _handlers.emplace(name, std::move(cmd));
    };
    register_handler("quit", &session::handle_quit, "quit");
    register_handler("exit", &session::handle_quit, "exit");
    register_handler("bye", &session::handle_quit, "bye");
    register_handler("get", &session::handle_get, "get <key-name>");
    register_handler("set", &session::handle_set, "set <key-name>=<key-value>");
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
        [this, self = shared_from_this()] (boost::system::error_code ec, std::size_t bytes_transferred)
        {
            if(ec)
            {
#ifdef TRACE
                std::cout << _ep << " read error: " << ec << std::endl;
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

    std::istringstream is(_msg);
    _msg.clear();

    std::string cmdname;
    is >> cmdname;

    if(cmdname.empty())
    {
        write("invalid command");
        return;
    }

    auto it = _handlers.find(cmdname);
    if(it == _handlers.end())
    {
        write("command not found");
        return;
    }

    const command& cmd = it->second;

    try
    {
        return (this->*(cmd._handler))(is);
    }
    catch(const std::exception& e)
    {
        std::ostringstream os;
        os << "invalid '" << cmdname << "' command, usage: " << cmd._usage;
        write(os.str());
    }
}

void session::handle_quit(std::istream&)
{
    write("bye", [this, self = shared_from_this()]() {
        _s.close();
    });
}

void session::handle_get(std::istream& is)
{
    std::string key;
    is >> key;

    if(key.empty())
    {
        write("invalid 'get' command, usage: get <key-name>");
        return;
    }

    auto v = _server.get(key);
    if(!v)
    {
        std::ostringstream os;
        os << "key '" << key << "' not found";
        write(os.str());
        return;
    }

    std::ostringstream os;
    os << v->value << std::endl;
    os << "reads=" << v->reads << std::endl;
    os << "writes=" << v->writes;
    write(os.str());
}

void session::handle_set(std::istream& is)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(is, pt);

    if(pt.empty())
        throw std::invalid_argument("");

    const auto& kv = pt.front();
    const std::string& key = kv.first;
    const std::string& value = kv.second.data();
    auto v = _server.put(key, value);

    std::ostringstream os;
    os << v.value << std::endl;
    os << "reads=" << v.reads << std::endl;
    os << "writes=" << v.writes;
    write(os.str());
 }

void session::write(std::string s, std::function<void()> completion)
{
    s += "\n";
    std::shared_ptr<const std::string> ps = std::make_shared<std::string>(std::move(s));
    asio::async_write(_s, asio::buffer(*ps),
        [this, self = shared_from_this(), ps, completion]
        (boost::system::error_code ec, std::size_t bytes_transferred)
        {
            if(ec)
            {
#ifdef TRACE
                std::cout << _ep << " write error: " << ec << std::endl;
#endif
                _s.close();
                return;
            }

            if(completion)
                completion();
        });
}
