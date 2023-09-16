#pragma once

#include "http_server.h"
#include "json_serializer.h"
#include "model.h"

#include <string>
#include <string_view>
#include <boost/json.hpp>
#include <boost/algorithm/string/split.hpp> 

#define BOOST_BEAST_USE_STD_STRING_VIEW

namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;
    using namespace json_serialize;
    using namespace std::literals;

    template <typename Body, typename Allocator>
    using HttpRequest = http::request<Body, http::basic_fields<Allocator>>;

    template <typename Body, typename Allocator>
    using HttpResponse = http::response<Body, http::basic_fields<Allocator>>;

    template<typename Body, typename Allocator>
    HttpResponse<Body, Allocator> MakeHttpResponse(http::status status, std::string_view body,
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
            if (request.method() == http::verb::get) {
                send(ProcessGetResponse<Body, Allocator>(std::move(request), target));
            }
        }

    private:
        template<typename Body, typename Allocator>
        HttpResponse<Body, Allocator> ProcessGetResponse(http::request<Body, http::basic_fields<Allocator>>&& request, std::string_view target) {
            std::string_view head_of_url = "/api/v1/maps"sv;
            if (target == head_of_url) { //Хотим отправить все карты
                return MakeHttpResponse<Body, Allocator>(
                    http::status::ok,
                    json::serialize(SerializeAllMaps(game_.GetMaps())),
                    request.version(),
                    request.keep_alive(),
                    ContentType::TEXT_JSON
                );
            }
            else if (target.starts_with(head_of_url)) { //Хотим отправить конкретную карту
                std::string map_name(target.begin() + head_of_url.size() + 1, target.end());

                auto first_slash = map_name.find('/');
                if (first_slash != map_name.size()) { //Пока не известный Get запрос
                    return MakeHttpResponse<Body, Allocator>(
                        http::status::bad_request,
                        json::serialize(SerializeError("badRequest"sv, "Bad request"sv)),
                        request.version(),
                        request.keep_alive(),
                        ContentType::TEXT_JSON
                    );
                }

                auto map_id = model::Map::Id(std::string{ map_name });
                const auto* map_ptr = game_.FindMap(map_id);

                if (map_ptr != nullptr) { //Юху - карта нашлась
                    return MakeHttpResponse<Body, Allocator>(
                        http::status::ok,
                        json::serialize(SerializeCurrentMap(*map_ptr)),
                        request.version(),
                        request.keep_alive(),
                        ContentType::TEXT_JSON
                    );
                }
                else { //Таковой карты нет в БД
                    return MakeHttpResponse<Body, Allocator>(
                        http::status::not_found,
                        json::serialize(SerializeError("mapNotFound"sv, "Map not found"sv)),
                        request.version(),
                        request.keep_alive(),
                        ContentType::TEXT_JSON
                    );
                }

            }
            else { //Хотим отправить bad_request
                return MakeHttpResponse<Body, Allocator>(
                    http::status::bad_request,
                    json::serialize(SerializeError("badRequest"sv, "Bad request"sv)),
                    request.version(),
                    request.keep_alive(),
                    ContentType::TEXT_JSON
                );
            }
        }

    private:
        model::Game& game_;
    };

}  // namespace http_handler
