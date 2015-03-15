#pragma once

#define BOOST_ASIO_DISABLE_BUFFER_DEBUGGING
#define BOOST_ASIO_NO_TYPEID

// Via: Boost.Asio
#include <boost/asio.hpp>

// Augment
namespace boost
{
    // Enhance
    namespace asio
    {
        // Put them in their proper namespace
        typedef boost::system::error_code error_code;
    };
};

// Aliasing magic
namespace asio = boost::asio;
