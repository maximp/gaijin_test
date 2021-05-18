#include "config.hpp"



class server
{
public:
    server(asio::io_service& ios, unsigned short port);
    void accept();

    std::optional<data> get(const std::string& key) const;
    void put(const std::string& key, const std::string& value);

private:
    using data_t = std::unordered_map<std::string, data>;

    asio::ip::tcp::acceptor     _acceptor;
    asio::io_service::strand    _write_strand;
    asio::steady_timer          _write_timer;
    data_t                      _data;
};
