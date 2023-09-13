#pragma once

#include "json_serializer.h"

namespace json_serialize {
	namespace json = boost::json;

	json::value SerializeError(std::string_view code = "mapNotFound", std::string_view message = "Map not found") {
		json::object error_object = {
			{"code", code},
			{"message", message}
		};
		return json::value(std::move(error_object));
	}

	json::value SerializeAllMaps(const model::Game::Maps& maps) {
		json::array array_of_head_map;

		for (const auto& map : maps) {
			json::object head_current_map = {
				{"id", *(map.GetId())},
				{"name", map.GetName()}
			};
			array_of_head_map.push_back(head_current_map);
		}
		return json::value(std::move(array_of_head_map));
	}

	json::value SerializeCurrentMap(const model::Map& map) {
		json::object map_object = {
			{"id ", *(map.GetId())},
			{"name", map.GetName()}
		};

		//Сериализируем дороги, строения и офисы

		auto CreateJsonArray = [](auto&& object) -> json::array {
			json::array array;
			for (const auto& element : object) {
				array.push_back(element);
			}
			return array;
		};

		map_object["roads"] = CreateJsonArray(map.GetRoads());
		map_object["buildings"] = CreateJsonArray(map.GetBuildings());
		map_object["offices"] = CreateJsonArray(map.GetOffices());

		return json::value(std::move(map_object));
	}

	//Всмопомогательная функция для создания значения "дороги"
	json::value SerializeRoads(const model::Road& road) {
		const auto& [x0, y0] = road.GetStart();
		const auto& [x1, y1] = road.GetEnd();

		json::object road_object = {
			{"x0", x0},
			{"y0", y0}
		};

		if (road.IsVertical()) road_object["y1"] = y1;
		else road_object["x1"] = x1;

		return json::value(std::move(road_object));
	}

	//Вспомогательная функция для создания значения "строения"
	json::value SerializeBuildings(const model::Building& building) {
		const auto& [position, size] = building.GetBounds();

		json::object building_object = {
			{"x", position.x},
			{"y", position.y},
			{"w", size.width},
			{"h", size.height}
		};

		return json::value(std::move(building_object));
	}

	//Вспомогательная функция для создания значения "офисы"
	json::value SerializeOffices(const model::Office& office) {
		const auto& id = office.GetId();
		const auto& position = office.GetPosition();
		const auto& offset = office.GetOffset();

		json::object office_object = {
			{"id", *id},
			{"x", position.x},
			{"y", position.y},
			{"offsetX", offset.dx},
			{"offsetY", offset.dy}
		};

		return json::value(std::move(office_object));
	}

} // namespace json_serializer
