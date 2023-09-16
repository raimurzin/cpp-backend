#pragma once

#include "model.h"
#include <boost/json.hpp>
#include <string_view>

namespace json_serialize {
	boost::json::value SerializeError(std::string_view code, std::string_view message); //Создаем возвращаемое значение в виде ошибки

	boost::json::value SerializeAllMaps(const model::Game::Maps& maps); //Создаем возвращаемое значени в виде списка карт
	boost::json::value SerializeCurrentMap(const model::Map& map); //Создаем возвращаемое значение в виде конкретной карты

	boost::json::value SerializeRoads(const model::Road& road); //Всмопомогательная функция для создания значения "дороги"
	boost::json::value SerializeBuildings(const model::Building& building); //Вспомогательная функция для создания значения "строения"
	boost::json::value SerializeOffices(const model::Office& office); //Вспомогательная функция для создания значения "офисы"
}  // namespace json_loader
