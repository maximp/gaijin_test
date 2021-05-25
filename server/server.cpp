#include "server.hpp"
#include "session.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

server::server(asio::io_service& ios, unsigned short port)
:   _acceptor(ios, { asio::ip::tcp::v4(), port }),
    _modified(false)
{
    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.listen();
}

void server::start()
{
    accept();
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

void server::read(std::istream& is)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(is, pt);

    std::unique_lock guard(_lock);

    _data.clear();
    for(const auto& kv : pt)
    {
        const std::string& key = kv.first;
        const std::string& value = kv.second.data();
        _data.emplace(key, value);
    }

    _modified = false;
}

void server::write(std::ostream& os) const
{
    std::shared_lock guard(_lock);
    for(const auto& kv : _data)
        os << kv.first << "=" << kv.second.value << std::endl;
    _modified = false;
}

std::optional<server::value_t> server::get(const std::string& key) const
{
    std::shared_lock guard(_lock);

    auto it = _data.find(key);
    if(it == _data.end())
        return {};

    data_t& d = const_cast<data_t&>(it->second);

    ++d.reads;
    ++_stats.overall.n_get;
    ++_stats.last.n_get;

    return {d.as_value()};
}

server::value_t server::put(const std::string& key, const std::string& value)
{
    std::unique_lock guard(_lock);

    auto p = _data.emplace(key, value);
    auto it = p.first;
    data_t& d = it->second;
    if(!p.second)
        d.value = value;

    ++d.writes;
    ++_stats.overall.n_set;
    ++_stats.last.n_set;
    _modified = true;

    return d.as_value();
}

server::stats_t server::stats(bool reset_last)
{
    std::unique_lock guard(_lock);
    server::stats_t result = _stats;
    if(reset_last)
        _stats.last = stat_t();
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, server::stats_t& s)
{
    return os << "Overall " << s.overall << ", last " << s.last;
}

std::ostream& operator<<(std::ostream& os, server::stat_t& s)
{
    return os << "(get: " << s.n_get << ", set: " << s.n_set << ")";
}
