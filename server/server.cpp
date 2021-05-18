#include "server.hpp"
#include "session.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

server::server(asio::io_service& ios, unsigned short port)
:   _acceptor(ios, { asio::ip::tcp::v4(), port })
{
    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.listen();
}

void server::accept()
{
    _acceptor.async_accept([this](boost::system::error_code ec, asio::ip::tcp::socket socket)
    {
        if(!ec)
        {
            auto s = std::make_shared<session>(std::move(socket));
            s->start();
        }

        accept();
    });
}