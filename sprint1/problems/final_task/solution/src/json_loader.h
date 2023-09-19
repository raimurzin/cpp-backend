#pragma once

#include <filesystem>
#include <boost/json.hpp>
#include "model.h"

namespace json_loader {
	class LoadGame {
	public:
		LoadGame() = default;
		LoadGame(const LoadGame&) = delete;
		LoadGame& operator=(const LoadGame&) = delete;

		model::Game operator()(const std::filesystem::path& json_path);
	private:
		static std::string CastBoostString(const boost::json::string& boost_string);
		static void LoadRoads(model::Map& map, boost::json::array& array_of_roads);
		static void LoadBuildings(model::Map& map, boost::json::array& array_of_buildings);
		static void LoadOffices(model::Map& map, boost::json::array& array_of_offices);
	};
}  // namespace json_loader
