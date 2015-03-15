#pragma once

/**
 * Note about pre-compiled dependencies: This module is designed
 * to allow usage on both C++03 and C++11 compilers
 * without big changes. Because the Boost library is
 * compatible (API-wise and namespace aliasing-wise) with
 * the standard, we've chosen it as a layer for C++03 compilers
 * that do not have support for the new C++11 features. It is
 * advisable that all projects update to the new standard and
 * take advantage of the new shared_ptr, allocate_shared, locks
 * guards, atomics and new stuff the standard brings, while
 * still having a backwards compatible layer through
 * the Boost library and its implementation.
 */

// Use cpp11::threads on Windows
#if defined (WIN32) || defined (_WIN32) || defined (OS_IPHONE)
    #define BOOST_THREAD_NO_LIB
    #define BOOST_SYSTEM_NO_LIB
#endif

// For __uncaught_exception
#if defined (WIN32) || defined (_WIN32)
    // A few requires
    #include <exception>
    #include <direct.h>
    #include <eh.h>
    
    // We're threaded on Windows
    #define BOOST_HAS_WINTHREADS
#endif

// Determine if C++03/C++11
#include <boost/config.hpp>

// Overwrite boost_config and comment-out the BOOST_DISABLE_THREADS macro
#undef BOOST_DISABLE_THREADS
#define BOOST_HAS_THREADS

// Enable it back for Android and IOS
#if defined (OS_ANDROID) || defined (OS_IPHONE)
    #define BOOST_THREAD_POSIX
    #define BOOST_HAS_PTHREADS
#endif

// Check for C++03/C++11 differences
#if !(defined (BOOST_NO_CXX11_HDR_THREAD)) && !(defined (BOOST_NO_CXX11_HDR_MUTEX)) && !(defined (BOOST_NO_CXX11_HDR_CONDITION_VARIABLE))
    // C++11 features
    #include <atomic>
    #include <thread>
    #include <mutex>
    #include <condition_variable>
    #include <unordered_map>
    
    // Flag for C++11 compilation
    #define _ASYNC_SERVER_COMPILED_WITH_CPP11
    
    // Aliasing
    namespace cpp11 = std;
#else
    // For C++03, use Boost
    #include <boost/atomic.hpp>
    #include <boost/thread.hpp>
    #include <boost/thread/mutex.hpp>
    #include <boost/thread/condition_variable.hpp>
    #include <boost/unordered_map.hpp>
    
    // Aliasing
    namespace cpp11 = boost;
#endif

