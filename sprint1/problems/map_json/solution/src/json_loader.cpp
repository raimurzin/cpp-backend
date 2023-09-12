#include "json_loader.h"

#include <fstream>
#include <iostream>
#include <string>

namespace json_loader {
    using namespace model;
    using namespace std::literals;
    namespace json = boost::json;

    void LoadRoads(Map& map, const boost::json::array& array_of_roads) {
        for (const auto& current_road : array_of_roads) {
            const auto& road_object = current_road.as_object();

            int start_x = road_object.at("x0"sv).as_int64();
            int start_y = road_object.at("y0"sv).as_int64();

            if (road_object.count("x1"sv)) {
                int end_x = road_object.at("x1"sv).as_int64();
                map.AddRoad( 
                    Road{ Road::HORIZONTAL, Point { start_x, start_y }, end_x } );
            }
            else {
                int end_y = road_object.at("y1"sv).as_int64();
                map.AddRoad(
                    Road{ Road::VERTICAL, Point { start_x, start_y }, end_y } );
            }
        }
    }

    void LoadBuildings(Map& map, const boost::json::array& array_of_buildings) {
        for (const auto& current_building : array_of_buildings) {
            const auto& building_object = current_building.as_object();

            int x = building_object.at("x"sv).as_int64();
            int y = building_object.at("y"sv).as_int64();

            int width = building_object.at("w"sv).as_int64();
            int height = building_object.at("h"sv).as_int64();

            map.AddBuilding(Building{
                Rectangle { Point { x, y }, Size { width, height } } });
        }

    }

    void LoadOffices(Map& map, const boost::json::array& array_of_offices) {
        auto get_std_string = [](boost::json::string json_string) -> std::string {
            return { json_string.begin(), json_string.end() };
        };

        for (const auto& current_office : array_of_offices) {
            const auto& office_object = current_office.as_object();

            auto id = get_std_string(office_object.at("id"sv).as_string());

            int x = office_object.at("x"sv).as_int64();
            int y = office_object.at("y"sv).as_int64();

            int x_offset = office_object.at("offsetX"sv).as_int64();
            int y_offset = office_object.at("offsetY"sv).as_int64();

            map.AddOffice(Office {
                Office::Id(id),
                Point { x, y },
                Offset { x_offset, y_offset }
                });
        }
    }

    Game LoadGame(const std::filesystem::path& json_path) {
        Game game;

        std::ifstream stream(json_path);
        std::stringstream buffer;
        buffer << stream.rdbuf();

        auto obj = json::parse(buffer.str()).as_object();

        auto get_std_string = [](boost::json::string json_string) -> std::string {
            return { json_string.begin(), json_string.end() };
        };

        const auto& maps = obj.at("maps"sv).as_array();
        for (const auto& map_element : maps) {
            const auto& current_object = map_element.as_object();

            auto map_id = get_std_string(current_object.at("id"sv).as_string());
            auto map_name = get_std_string(current_object.at("name"sv).as_string());
            
            Map map(Map::Id(map_id), map_name);

            LoadRoads(map, current_object.at("roads"sv).as_array());
            LoadBuildings(map, current_object.at("buildings"sv).as_array());
            LoadOffices(map, current_object.at("offices"sv).as_array());
        
            game.AddMap(std::move(map));
        }

        return game;
    }

}  // namespace json_loader
