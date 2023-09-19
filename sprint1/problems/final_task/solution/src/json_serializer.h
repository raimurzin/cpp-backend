#pragma once

#include "model.h"
#include <boost/json.hpp>
#include <string_view>

namespace json_serializer {
	class SerializeResponse {
	public:
		SerializeResponse() = default;
		SerializeResponse(const SerializeResponse&) = delete;
		SerializeResponse& operator=(const SerializeResponse&) = delete;

	private:
		static boost::json::value SerializeRoads(const model::Road& road); //Всмопомогательная функция для создания значения "дороги"
		static boost::json::value SerializeBuildings(const model::Building& building); //Вспомогательная функция для создания значения "строения"
		static boost::json::value SerializeOffices(const model::Office& office); //Вспомогательная функция для создания значения "офисы"

	public:
		boost::json::value operator()(std::string_view code, std::string_view message); //Создаем возвращаемое значение в виде ошибки
		boost::json::value operator()(const model::Game::Maps& maps); //Создаем возвращаемое значени в виде списка карт
		boost::json::value operator()(const model::Map& map); //Создаем возвращаемое значение в виде конкретной карты
	};
}  // namespace json_loader
