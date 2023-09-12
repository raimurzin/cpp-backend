#include "json_serializer.h"

namespace json_serializer {
	namespace json = boost::json;
	using namespace std::literals;

    //Отправка объекта - "ошибка" клиенту
    json::value SerializeError(std::string_view code, std::string_view message) {
        json::object object;
        object["code"sv] = code;
        object["message"sv] = message;
        return json::value(std::move(object));
    }

    //Создание объекта "Дорога"
    json::value Serialize(const Road& road) {
        json::object object;

        const auto& [start_x, start_y] = road.GetStart();
        object["x0"sv] = start_x;
        object["y0"sv] = start_y;

        const auto& [end_x, end_y] = road.GetEnd();
        if (road.IsVertical()) object["y1"sv] = end_y;
        else object["x1"sv] = end_x;

        return json::value(std::move(object));
    }

    //Создание объекта "строения"
    json::value Serialize(const Building& building) {
        json::object object;

        const auto& [position, size] = building.GetBounds();
        object["x"sv] = position.x;
        object["y"sv] = position.y;

        object["w"sv] = size.width;
        object["h"sv] = size.height;

        return json::value(std::move(object));
    }

    //Создание объекта "офис"
    json::value Serialize(const Office& office) {
        json::object object;

        const auto& id = office.GetId();
        const auto& position = office.GetPosition();
        const auto& offset = office.GetOffset();

        object["id"sv] = *id;
        object["x"sv] = position.x;
        object["y"sv] = position.y;
        object["offsetX"sv] = offset.dx;
        object["offsetY"sv] = offset.dy;

        return json::value(std::move(object));
    }

    //Отправка объекта - "список карт" клиенту
    json::value Serialize(const Game::Maps& maps) {
        json::array maps_array;

        for (const auto& map : maps) {
            json::object res_object;
            res_object["id"sv] = *map.GetId();
            res_object["name"sv] = map.GetName();
            maps_array.push_back(res_object);
        }

        return json::value(std::move(maps_array));
    }

    //Отправка объекта - "кокнрентная карта" клиенту
    json::value Serialize(const Map& map) {
        json::object object;

        object["id"] = *map.GetId();
        object["name"] = map.GetName();

        json::array array_of_roads;
        for (const auto& road : map.GetRoads()) {
            array_of_roads.push_back(Serialize(road));
        }
        object["roads"] = std::move(array_of_roads);

        json::array array_of_buildings;
        for (const auto& building : map.GetBuildings()) {
            array_of_buildings.push_back(Serialize(building));
        }
        object["buildings"] = std::move(array_of_buildings);

        json::array array_of_offices;
        for (const auto& office : map.GetOffices()) {
            array_of_offices.push_back(Serialize(office));
        }
        object["offices"] = std::move(array_of_offices);

        return json::value(std::move(object));
    }

} //namespace json_serializer
