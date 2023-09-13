#pragma once
#include "http_server.h"
#include "json_serializer.h"
#include "model.h"

#include <string_view>
#include <boost/json.hpp>

namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;
    using namespace json_serialize;
    using namespace std::literals;

    using StringRequest = http::request<http::string_body>;
    using StringResponse = http::response<http::string_body>;

    StringResponse MakeStringResponse(http::status status, std::string_view body,
        unsigned http_version, bool keep_alive, std::string_view content_type);

    class RequestHandler {
    public:
        explicit RequestHandler(model::Game& game) : game_{ game } {}

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        struct ContentType {
            ContentType() = delete;
            constexpr static std::string_view TEXT_HTML = "text/html"sv;
            constexpr static std::string_view TEXT_JSON = "application/json"sv;
            // При необходимости внутрь ContentType можно добавить и другие типы контента
        };

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& request, Send&& send) {
            auto target = request.target();
            std::string_view url_head = "/api/v1/maps"sv;

            if (target == url_head) {
                send(MakeStringResponse(
                    http::status::ok,
                    json::serialize(SerializeAllMaps(game_.GetMaps())),
                    request.version(),
                    request.keep_alive(),
                    ContentType::TEXT_JSON
                ));
            }
            else if (target.starts_with(url_head) && !target.ends_with("/"sv)) {
                std::string_view id = target.substr(url_head.size() + 1);
                auto map_id = model::Map::Id{ std::string{id} };
                const auto* map_ptr = game_.FindMap(map_id);

                if (map_ptr != nullptr)
                    send(MakeStringResponse(
                        http::status::ok,
                        json::serialize(SerializeCurrentMap(*map_ptr)),
                        request.version(),
                        request.keep_alive(),
                        ContentType::TEXT_JSON
                    ));
                else
                    send(MakeStringResponse(
                        http::status::not_found, 
                        json::serialize(SerializeError("mapNotFound"sv, "Map not found")),
                        request.version(),
                        request.keep_alive(),
                        ContentType::TEXT_JSON
                    ));
            }
            else if (target.starts_with("/api/"sv)) {
                send(MakeStringResponse(
                    http::status::bad_request,
                    json::serialize(SerializeError("badRequest"sv, "Bad request")),
                    request.version(),
                    request.keep_alive(),
                    ContentType::TEXT_JSON
                ));
            }
        }

    private:
        model::Game& game_;
    };

}  // namespace http_handler
