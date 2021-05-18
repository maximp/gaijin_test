#include "server.hpp"
#include "session.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

server::server(asio::io_service& ios, unsigned short port)
:   _acceptor(ios, { asio::ip::tcp::v4(), port }),
    _write_strand(ios),
    _write_timer(ios)
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
            auto s = std::make_shared<session>(*this, std::move(socket));
            s->start();
        }

        accept();
    });
}

std::optional<data> server::get(const std::string& key) const
{
    return {};
}

void server::put(const std::string& key, const std::string& value)
{

}