#pragma once

#include <boost/asio.hpp>

#include <atomic>

#if !defined(BOOST_ASIO_STANDALONE)
namespace asio = boost::asio;
#endif
