#include "json_loader.h"
#include <fstream>
#include <string>

namespace json_loader {
    using namespace model;
    namespace json = boost::json;
    using namespace std::literals;

    Game LoadGame::operator()(const std::filesystem::path& json_path) {
        Game game;

        std::ifstream incoming_stream(json_path);
        if (!incoming_stream.is_open()) {
            throw std::invalid_argument("Could not open file!");
        }
        std::string json((std::istreambuf_iterator<char>(incoming_stream)),
            std::istreambuf_iterator<char>());

        auto json_object = json::parse(json).as_object();
        const auto array_of_maps = json_object.at("maps").as_array();

        for (const auto& map : array_of_maps) {
            auto map_object = map.as_object();

            auto id = CastBoostString(map.at("id").as_string());
            auto name = CastBoostString(map.at("name").as_string());

            Map recieved_map(Map::Id{ id }, name);

            LoadRoads(recieved_map, map_object.at("roads").as_array());
            LoadBuildings(recieved_map, map_object.at("buildings").as_array());
            LoadOffices(recieved_map, map_object.at("offices").as_array());

            game.AddMap(std::move(recieved_map));
        }

        return game;
    }

    std::string LoadGame::CastBoostString(const json::string& boost_string) {
        return { boost_string.begin(), boost_string.end() };
    }

    void LoadGame::LoadRoads(Map& map, boost::json::array& array_of_roads) {
        for (const auto& road : array_of_roads) {
            auto road_object = road.as_object();

            int x0 = static_cast<int>(road_object.at("x0").as_int64());
            int y0 = static_cast<int>(road_object.at("y0").as_int64());

            if (road_object.contains("x1")) {
                int x1 = static_cast<int>(road_object.at("x1").as_int64());
                map.AddRoad(Road{ Road::HORIZONTAL, Point { x0, y0 }, x1 });
            }
            else if (road_object.contains("y1")) {
                int y1 = static_cast<int>(road_object.at("y1").as_int64());
                map.AddRoad(Road{ Road::VERTICAL, Point { x0, y0 }, y1 });
            }
        }
    }

    void LoadGame::LoadBuildings(Map& map, boost::json::array& array_of_buildings) {
        for (const auto& building : array_of_buildings) {
            auto building_object = building.as_object();

            int x = static_cast<int>(building_object.at("x").as_int64());
            int y = static_cast<int>(building_object.at("y").as_int64());
            int w = static_cast<int>(building_object.at("w").as_int64());
            int h = static_cast<int>(building_object.at("h").as_int64());

            map.AddBuilding(Building(Rectangle{ Point { x, y }, Size { w, h } }));
        }
    }

    void LoadGame::LoadOffices(Map& map, boost::json::array& array_of_offices) {
        for (const auto& office : array_of_offices) {
            auto office_object = office.as_object();

            std::string id = CastBoostString(office_object.at("id").as_string());
            int x = static_cast<int>(office_object.at("x").as_int64());
            int y = static_cast<int>(office_object.at("y").as_int64());

            int x_offset = static_cast<int>(office_object.at("offsetX").as_int64());
            int y_offset = static_cast<int>(office_object.at("offsetY").as_int64());

            map.AddOffice(Office{ Office::Id(id), Point { x, y }, Offset { x_offset, y_offset } });
        }
    }

}  // namespace json_loader
