#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
namespace request_handler
{

    class RequestHandler
    {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue &db,
                       const map_renderer::MapRenderer &renderer)
            : db_(db), renderer_(renderer){};
        // Возвращает информацию о маршруте (запрос Bus)
        domain::BusStats GetBusStats(std::string bus_name) const;

        // Возвращает маршруты, проходящие через
        domain::StopStats GetStopStats(std::string stop_name) const;

        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue &db_;
        const map_renderer::MapRenderer &renderer_;
    };
}