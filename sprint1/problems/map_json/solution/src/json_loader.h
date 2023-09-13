#pragma once

#include <filesystem>

#include "model.h"

namespace json_loader {
	class LoadGame {
	public:
		model::Game operator()(const std::filesystem::path& json_path);
	private:
		std::string CastBoostString(const boost::json::string& boost_string);
		void LoadRoads(Map& map, boost::json::array& array_of_roads);
        void LoadBuildings(Map& map, boost::json::array& array_of_buildings);
        void LoadOffices(Map& map, boost::json::array& array_of_offices);
	};
}  // namespace json_loader
