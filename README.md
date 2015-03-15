# Server-Client-Async-HTTP-Protocol

Basic HTTP Protocol with work queue on client(worker) side from server requests.

It's implementation is based mostly on asio c++ library which provides consistent asynchronous model for network and I/O operations.

We have Boost Libraries as dependency for emulating cpp11 where we find a non c++ 11 compiler and also for other utilites such as lexical cast, bind.
