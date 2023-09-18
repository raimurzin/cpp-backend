#include <syncstream>
#include <boost/asio/dispatch.hpp>

#include "http_server.h"

namespace http_server {
    void ReportError(beast::error_code ec, std::string_view what) {
        std::osyncstream(std::cout) << what << ": "sv << ec.message() << std::endl;
    }

    void SessionBase::Run() {
        net::dispatch(stream_.get_executor(), beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
    }
}  // namespace http_server
