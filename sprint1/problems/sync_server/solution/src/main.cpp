#ifdef WIN32
#include <sdkddkver.h>
#endif

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include <optional>
#include <syncstream>
#include <sstream>
#include <functional>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;

using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;

std::optional<StringRequest> ReadRequest(tcp::socket& socket, beast::flat_buffer& buffer) {
    beast::error_code ec;
    StringRequest request;
    http::read(socket, buffer, request, ec); //Блокрирующая функция

    if (ec == http::error::end_of_stream) {
        return std::nullopt;
    }
    if (ec) {
        throw std::runtime_error("Failed to read request: "s.append(ec.message()));
    }
    return request;
}

void DumpRequest(const StringRequest& req) {
    std::cout << req.method_string() << ' ' << req.target() << std::endl;
    for (const auto& header : req) {
        std::cout << "  "sv << header.name_string() << ": "sv << header.value() << std::endl;
    }
}

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

StringResponse GetMethodResponse(http::status status, std::string_view body, unsigned http_version,
                        bool keep_alive, std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse HeadMethodResponse(http::status status, size_t body_size, unsigned http_version,
                        bool keep_alive, std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.content_length(body_size);
    response.keep_alive(keep_alive);
    return response;
}

StringResponse OtherMethodsReponse(http::status status, std::string_view body, unsigned http_version,
                        bool keep_alive, std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.set(http::field::allow, "GET, HEAD");
    response.body() = body;
    response.content_length(response.body().size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse HandleRequest(StringRequest&& request) {
    switch (request.method()) {
        case http::verb::get: {
            std::stringstream text;
            text << "Hello, " << request.target().substr(1);
            return GetMethodResponse(http::status::ok, text.str(), request.version(), request.keep_alive());
        }
        case http::verb::head: {
            std::stringstream text;
            text << "Hello, " << request.target().substr(1);
            return HeadMethodResponse(http::status::ok, text.str().size(), request.version(), request.keep_alive());
        }
        default: {
            return OtherMethodsReponse(http::status::method_not_allowed, "Invalid method"sv, request.version(), request.keep_alive());
        }
    }
}

template<typename RequestHandler>
void HandleConnection(tcp::socket& socket, RequestHandler&& request_handler) {
    try {
        beast::flat_buffer buffer; //Динамический буфер для чтения сообщения

        while (auto request = ReadRequest(socket, buffer)) { //Принимаем сообщения пока клиент отправляет
            DumpRequest(*request);
            StringResponse response = HandleRequest(*std::move(request));
            http::write(socket, response);
            if (response.need_eof()) {
                break;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_send, ec); 	// Запрещаем дальнейшую отправку данных через сокет
}

int main() {
    const auto address = net::ip::make_address("0.0.0.0");
    constexpr unsigned short port = 8080;

    net::io_context io_context;
    tcp::acceptor acceptor(io_context, { address, port });

    while (true) {
        tcp::socket socket(io_context);
        std::cout << "Server has started..."sv << std::endl;
        acceptor.accept(socket);
        std::cout << "Connection received"sv << std::endl;

	std::thread t(
            [](tcp::socket socket) {
                HandleConnection(socket, HandleRequest); },
            std::move(socket));
        t.detach(); // После вызова detach поток продолжит выполняться независимо от объекта t
    }

    return 0;
}
