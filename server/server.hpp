#include "config.hpp"

#include <filesystem>
#include <optional>
#include <shared_mutex>

class server
{
public:
    server(asio::io_service& ios, unsigned short port);
    void start();

    struct value_t {
        std::string value;
        int reads;
        int writes;
    };

    void read(std::istream& is);
    void write(std::ostream& os) const;
    bool modified() const { return _modified; }

    std::optional<value_t> get(const std::string& key) const;
    value_t put(const std::string& key, const std::string& value);

private:
    void accept();

    struct data_t
    {
        data_t() {}

        data_t(const data_t&) = default;
        data_t& operator=(const data_t&) = default;

        data_t(const std::string& v) : value(v) {}
        data_t(std::string&& v) : value(std::move(v)) {}

        std::string     value;
        std::atomic_int reads = 0;
        std::atomic_int writes = 0;

        value_t as_value() const {
            value_t v;
            v.value = value;
            v.reads = reads;
            v.writes = writes;
            return v;
        }
    };

    using container_t = std::unordered_map<std::string, data_t>;

    asio::ip::tcp::acceptor     _acceptor;
    container_t                 _data;
    mutable std::shared_mutex   _lock;
    mutable std::atomic_bool    _modified;
};
