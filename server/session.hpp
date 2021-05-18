#include "config.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class server;

class session : public std::enable_shared_from_this<session>
{
public:
    session(server& srv, asio::ip::tcp::socket s);
    ~session();

    void start();

private:
    void async_read();
    void handle_command();
    void handle_quit(std::istream& is);
    void handle_get(std::istream& is);
    void handle_set(std::istream& is);

    void write(std::string s, std::function<void()> completion = {});

    using handler_t = void (session::*)(std::istream&);

    struct command {
        handler_t   _handler = nullptr;
        std::string _usage;
    };

    using handlers_t = std::unordered_map<std::string, command>;

    server&                  _server;
    asio::ip::tcp::socket    _s;
    asio::ip::tcp::endpoint  _ep;
    std::string              _msg;
    handlers_t               _handlers;
};
