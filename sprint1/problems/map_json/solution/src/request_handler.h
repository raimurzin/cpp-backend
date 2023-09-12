#pragma once
#include "http_server.h"
#include "model.h"
#include "json_serializer.h"

#include <boost/json.hpp>

namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    using namespace std::literals;
    using namespace model;
    using namespace json_serializer;

    using StringRequest = http::request<http::string_body>;
    using StringResponse = http::response<http::string_body>;

    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
        bool keep_alive, std::string_view content_type = "undefined"sv);

    class RequestHandler {
    public:
        explicit RequestHandler(Game& game) : game_{ game } {}

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        struct ContentType {
            ContentType() = delete;
            constexpr static std::string_view TEXT_HTML = "text/html"sv;
            constexpr static std::string_view TEXT_JSON = "application/json"sv;
            // При необходимости внутрь ContentType можно добавить и другие типы контента
        };

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            // Обработать запрос request и отправить ответ, используя send

            auto GetStringResponse = [&req](http::status status, json::value value) -> StringResponse {
                return MakeStringResponse(status, json::serialize(value), req.version(), req.keep_alive(), ContentType::TEXT_JSON);
            };

            auto target = req.target();
            std::string_view top_of_url = "/api/v1/maps";

            if (target == top_of_url) {
                send(GetStringResponse(http::status::ok, Serialize(game_.GetMaps())));
            }
            else if (target.starts_with(top_of_url) && !target.ends_with("/"sv)) {
                std::string_view id = target.substr(top_of_url.size() + 1);
                auto map_id = Map::Id{ std::string{id} };
                const auto* map_ptr = game_.FindMap(map_id);

                if (map_ptr != nullptr)
                    send(GetStringResponse(http::status::ok, Serialize(*map_ptr)));
                else
                    send(GetStringResponse(http::status::not_found, SerializeError("mapNotFound"sv, "Map not found")));
            }
            else if (target.starts_with("/api/"sv)) {
                send(GetStringResponse(http::status::bad_request, SerializeError("badRequest"sv, "Bad request")));
            }

        }

    private:
        Game& game_;
    };

}  // namespace http_handler
