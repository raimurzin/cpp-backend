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

    /*
            В OnRead в возможны три ситуации:
        Если клиент закрыл соединение, то сервер должен завершить сеанс.
        Если произошла ошибка чтения, выведите её в stdout.
        Если запрос прочитан без ошибок, делегируйте его обработку классу-наследнику.
    */
    void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
        if (ec == http::error::end_of_stream) { //Клиент закрыл соединение
            Close();
        }
        if (ec) {
            return ReportError(ec, "read"sv);
        }
        HandleRequest(std::move(request_));
    }

    void SessionBase::OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written) {
        if (ec) return ReportError(ec, "write"sv);
        if (close) return Close(); // Семантика ответа требует закрыть соединение
        Read(); // Считываем следующий запрос
    }

    void SessionBase::Close() {
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    }
}  // namespace http_server
