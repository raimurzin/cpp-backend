#pragma once

#include <filesystem>
#include <boost/json.hpp>

#include "model.h"

namespace json_loader {
	model::Game LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
