#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"

#include "json.h"
#include <vector>

namespace json_reader
{
    struct StatRequest;
}

namespace request_handler
{

    class RequestHandler
    {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue &db,
                       const map_renderer::MapRenderer &renderer,
                       const transport_router::TransportRouter &router)
            : db_(db), renderer_(renderer), router_{router} {}
        // Возвращает информацию о маршруте (запрос Bus)
        domain::BusStats GetBusStats(const std::string &bus_name) const;

        // Возвращает маршруты, проходящие через
        domain::StopStats GetStopStats(const std::string &stop_name) const;

        svg::Document RenderMap() const;

        std::optional<transport_router::Route> ConstructRoute(const std::string &from, const std::string &to) const
        {
            return router_.GetRoute(from, to);
        }

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue &db_;
        const map_renderer::MapRenderer &renderer_;
        const transport_router::TransportRouter &router_;
    };
    json::Document ProcessStatRequests(const RequestHandler &handler, const std::vector<std::unique_ptr<json_reader::StatRequest>> &requests);
}