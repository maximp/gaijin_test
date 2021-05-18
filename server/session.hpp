#include "config.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class session : public std::enable_shared_from_this<session>
{
public:
    session(asio::ip::tcp::socket s);
    ~session();

    void start();

private:
    void async_read();
    void handle_command();
    void handle_quit();
    void handle_get();
    void handle_set();

    void write(std::string s);

    struct command {
        std::function<void()> _handler;
        std::string _usage;
    };

    using handlers_t = std::unordered_map<std::string, command>;

    asio::ip::tcp::socket    _s;
    asio::ip::tcp::endpoint  _ep;
    std::string              _msg;
    std::vector<std::string> _parts;
    handlers_t               _handlers;
};
