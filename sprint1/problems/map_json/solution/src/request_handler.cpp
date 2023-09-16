#include "request_handler.h"

namespace http_handler {
    template<typename Body, typename Allocator>
    HttpResponse<Body, Allocator> MakeHttpResponse(http::status status, std::string_view body,
        unsigned http_version, bool keep_alive, std::string_view content_type) {
        HttpRequest<Body, Allocator> response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }
}  // namespace http_handler
