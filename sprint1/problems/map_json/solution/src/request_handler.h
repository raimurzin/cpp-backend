#pragma once
#include "http_server.h"
#include "json_serializer.h"
#include "model.h"

#include <string_view>
#include <boost/json.hpp>

namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    using namespace json_serialize;
    using namespace std::literals;

    using StringRequest = http::request<http::string_body>;
    using StringResponse = http::response<http::string_body>;

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
            auto json_response = [&request](http::status status, json::value value) {
                return MakeStringResponse(status, json::serialize(value),
                    request.version(), request.keep_alive(), ContentType::TEXT_JSON);
            };

            auto target = request.target();
            std::string_view endpoint = "/api/v1/maps";

            if (target == endpoint) {
                send(json_response(http::status::ok, Serialize(game_.GetMaps())));
            }
            else if (target.starts_with(endpoint) && !target.ends_with("/"sv)) {
                std::string_view id = target.substr(endpoint.size() + 1);
                auto map_id = Map::Id{ std::string{id} };
                const auto* map_ptr = game_.FindMap(map_id);

                if (map_ptr == nullptr)
                    send(json_response(http::status::not_found, SerializeError("mapNotFound"sv, "Map not found"sv)));
                else
                    send(json_response(http::status::ok, Serialize(*map_ptr)));
            }
            else if (target.starts_with("/api/"sv)) {
                send(json_response(http::status::bad_request, SerializeError("badRequest"sv, "Bad request"sv)));
            }
        }

    private:
        model::Game& game_;
    };

}  // namespace http_handler
