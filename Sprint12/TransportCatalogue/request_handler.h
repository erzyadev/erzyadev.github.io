#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

#include "json.h"
#include <vector>

namespace json_reader
{
    class StatRequest;
}

namespace request_handler
{

    class RequestHandler
    {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue &db,
                       const map_renderer::MapRenderer &renderer)
            : db_(db), renderer_(renderer){};
        // Возвращает информацию о маршруте (запрос Bus)
        domain::BusStats GetBusStats(const std::string &bus_name) const;

        // Возвращает маршруты, проходящие через
        domain::StopStats GetStopStats(const std::string &stop_name) const;

        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue &db_;
        const map_renderer::MapRenderer &renderer_;
    };
    json::Document ProcessStatRequests(const RequestHandler &handler, const std::vector<std::unique_ptr<json_reader::StatRequest>> &requests);
}