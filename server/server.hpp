#include "config.hpp"

class server
{
public:
    server(asio::io_service& ios, unsigned short port);
    void accept();

private:
    asio::ip::tcp::acceptor _acceptor;
};
