#pragma once

#include <boost/asio.hpp>

#if !defined(BOOST_ASIO_STANDALONE)
namespace asio = boost::asio;
#endif

struct data
{
    data() {}
    data(const std::string& v) : value(v) {}
    data(std::string&& v) : value(std::move(v)) {}

    std::string value;
    int reads = 0;
    int writes = 0;
};