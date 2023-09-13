#pragma once
#include "sdk.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <utility>

namespace http_server {

    namespace net = boost::asio;
    using tcp = net::ip::tcp;
    namespace beast = boost::beast;
    namespace http = beast::http;
    using namespace std::literals;
    namespace sys = boost::system;

    using HttpRequest = http::request<http::string_body>;
    using HttpResponse = http::response<http::string_body>;

    void ReportError(beast::error_code ec, std::string_view what);

    class SessionBase {
        // Напишите недостающий код, используя информацию из урока
    protected:
        explicit SessionBase(tcp::socket&& socket) : stream_(std::move(socket)) {}

    public:
        SessionBase(const SessionBase&) = delete;
        SessionBase& operator=(const SessionBase&) = delete;
        ~SessionBase() = default;

    public:
        void Run();
    private:
        // Обработку запроса делегируем подклассу
        virtual void HandleRequest(HttpRequest&& request) = 0;
        virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;

    private:
        void Read() {
            request_ = {}; //Очищаем запрос от прежнего значения(Метод SessionBase::Read() мог вызываться несколько раз подряда)
            stream_.expires_after(30s);
            http::async_read(stream_, buffer_, request_,
                //По окончании работы считывания буфера будет вызван привязанный хендлер
                beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
        }
        /*
        В OnRead в возможны три ситуации:
            Если клиент закрыл соединение, то сервер должен завершить сеанс.
            Если произошла ошибка чтения, выведите её в stdout.
            Если запрос прочитан без ошибок, делегируйте его обработку классу-наследнику.
    */
        void OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
            if (ec == http::error::end_of_stream) { //Клиент закрыл соединение
                Close();
            }
            if (ec) {
                return ReportError(ec, "read"sv);
            }
            HandleRequest(std::move(request_));
        }

        void Close() {
            beast::error_code ec;
            stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        }

    protected:
        template <typename Body, typename Fields>
        void Write(http::response<Body, Fields>&& response) {
            // Запись выполняется асинхронно, поэтому response перемещаем в область кучи
            auto safe_response = std::make_shared<http::response<Body, Fields>>(std::move(response));

            auto self = GetSharedThis();
            http::async_write(stream_, *safe_response,
                [safe_response, self](beast::error_code ec, std::size_t bytes_written) {
                    self->OnWrite(safe_response->need_eof(), ec, bytes_written);
                });
        }
    private:
        void OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written) {
            if (ec) return ReportError(ec, "write"sv);
            if (close) return Close(); // Семантика ответа требует закрыть соединение
            Read(); // Считываем следующий запрос
        }

    private:
        beast::tcp_stream stream_; //Сокет поддерживающий таймауты
        beast::flat_buffer buffer_; //Динамический буффер для хранения информации
        HttpRequest request_; //Прочитанные запрос
    };

    template <typename RequestHandler>
    class Session : public SessionBase, public std::enable_shared_from_this<Session<RequestHandler>> {
        // Напишите недостающий код, используя информацию из урока
    public:
        template <typename Handler>
        Session(tcp::socket&& socket, Handler&& request_handler)
            : SessionBase(std::move(socket))
            , request_handler_(std::forward<Handler>(request_handler)) {}

    private:
        std::shared_ptr<SessionBase> GetSharedThis() override {
            return this->shared_from_this();
        }
    private:
        void HandleRequest(HttpRequest&& request) override {
            // Захватываем умный указатель на текущий объект Session в лямбде,
            // чтобы продлить время жизни сессии до вызова лямбды.
            // Используется generic-лямбда функция, способная принять response произвольного типа
            request_handler_(std::move(request), [self = this->shared_from_this()](auto&& response) {
                self->Write(std::move(response)); });
        }

    private:
        RequestHandler request_handler_;
    };

    template <typename RequestHandler>
    class Listener : public std::enable_shared_from_this<Listener<RequestHandler>> {
        // Напишите недостающий код, используя информацию из урока
    public:
        Listener(net::io_context& io, const tcp::endpoint& endpoint, RequestHandler&& request_handler) :
            io_{ io }, acceptor_{ net::make_strand(io) }, request_handler_(std::forward<RequestHandler>(request_handler)) {
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(net::socket_base::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen(net::socket_base::max_listen_connections);
        }

        void Run() {
            DoAccept();
        }

    private:
        void DoAccept() {
            acceptor_.async_accept(net::make_strand(io_),
                beast::bind_front_handler(&Listener::OnAccept, this->shared_from_this()));
        }

        void OnAccept(sys::error_code ec, tcp::socket socket) {
            if (ec) {
                return ReportError(ec, "accept"sv);
            }
            AsyncRunSession(std::move(socket));
            DoAccept();
        }

        void AsyncRunSession(tcp::socket&& socket) {
            std::make_shared<Session<RequestHandler>>(std::move(socket), request_handler_)->Run();
        }

    private:
        net::io_context& io_; //Необходим для управления асинхронными операция
        tcp::acceptor acceptor_; //Принимает соединения клиентов
        RequestHandler request_handler_; //Обработчик запросов
    };

    template <typename RequestHandler>
    void ServeHttp(net::io_context& ioc, const tcp::endpoint& endpoint, RequestHandler&& handler) {
        // Напишите недостающий код, используя информацию из урока

        // При помощи decay_t исключим ссылки из типа RequestHandler,
        // чтобы Listener хранил RequestHandler по значению
        using MyListener = Listener<std::decay_t<RequestHandler>>;

        std::make_shared<MyListener>(ioc, endpoint, std::forward<RequestHandler>(handler))->Run();
    }

}  // namespace http_server
