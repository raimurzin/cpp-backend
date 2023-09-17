#include "http_server.h"
#include <boost/asio/dispatch.hpp>
#include <iostream>
#include <syncstream>

namespace http_server {
    void ReportError(beast::error_code ec, std::string_view what) {
        std::osyncstream os{ std::cout };
        os << what << ": "sv << ec.message() << std::endl;
    }

    void SessionBase::Run() {
        net::dispatch(stream_.get_executor(), beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
    }
}  // namespace http_server
