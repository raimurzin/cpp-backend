#pragma once

#include <filesystem>
#include <boost/json.hpp>
#include "model.h"

namespace json_loader {
	class LoadGame {
	public:
		model::Game operator()(const std::filesystem::path& json_path);
	private:
		std::string CastBoostString(const boost::json::string& boost_string);
		void LoadRoads(model::Map& map, boost::json::array& array_of_roads);
        void LoadBuildings(model::Map& map, boost::json::array& array_of_buildings);
        void LoadOffices(model::Map& map, boost::json::array& array_of_offices);
	};
}  // namespace json_loader
