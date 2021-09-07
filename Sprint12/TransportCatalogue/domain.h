#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
#include <string>
#include <vector>
#include "geo.h"
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace domain
{

    struct Bus
    {
        Bus(std::string new_bus_number, std::vector<std::string> new_stops = {}, bool new_isLoop = false)
            : bus_number{move(new_bus_number)}, stops{move(new_stops)}, isLoop{new_isLoop} {}

        std::string bus_number;
        std::vector<std::string> stops;
        bool isLoop;
    };

    struct Stop
    {
        std::string stop_name;
        geo::Coordinates coordinates;
        std::unordered_set<std::string> buses;
        std::unordered_map<std::string, int> distances;
    };

    struct BusData
    {
        int total_stops;
        int unique_stops;
        double route_length;
        double curvature;
    };

    struct BusStats
    {
        std::string bus_number;
        std::optional<BusData> bus_stats;
    };

    struct StopData
    {
        const std::vector<std::string> buses;
    };

    struct StopStats
    {
        std::string stop_name;
        std::optional<StopData> stop_data;
    };

}