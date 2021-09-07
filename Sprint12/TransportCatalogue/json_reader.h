#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
// напишите решение с нуля
// код сохраните в свой git-репозиторий

//#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "json.h"
#include "map_renderer.h"
#include <algorithm>
#include <sstream>
#include <charconv>
#include <map>
#include <string>

using std::string_literals::operator""s;

namespace request_handler
{
    class RequestHandler;
}

namespace json_reader
{

    enum class InputQueryType
    {
        Bus,
        Stop,
    };
    struct AddQueries
    {
        std::vector<domain::Stop> stops;
        std::vector<domain::Bus> buses;
    };
    enum class StatRequestType
    {
        BUS_REQUEST,
        STOP_REQUEST,
        MAP_REQUEST,
        ROUTE_REQUEST
    };

    struct StatRequest
    {
        int id;
        StatRequest(int id) : id(id) {}
        virtual json::Node GetProcessed(const request_handler::RequestHandler &handler) const = 0;
        virtual ~StatRequest() = default;
    };

    geo::Coordinates ReadCoordinates(std::string &input);

    AddQueries ParseCreation(const std::vector<std::string> &inputs);

    int ReadNumber(std::istream &input);

    void StatsRequest(const transport_catalogue::TransportCatalogue &catalogue, std::string raw_query);

    struct JsonReadResult
    {
        std::vector<domain::Stop> new_stops;
        std::vector<domain::Bus> new_buses;
        std::vector<std::unique_ptr<StatRequest>> stat_requests;
        map_renderer::RenderSettings render_settings;
        transport_router::RoutingSettings routing_settings;
    };

    map_renderer::RenderSettings ReadRenderSetting(json::Node render_settings_node);

    JsonReadResult ReadJson(std::istream &input);

    json::Node MakeBusStatsNode(const std::optional<domain::BusData> &bus_data, int request_id);
    json::Node MakeStopStatsNode(const std::optional<domain::StopData> &stop_data, int request_id);
    json::Node MakeMapNode(svg::Document rendered_map, int request_id);
}