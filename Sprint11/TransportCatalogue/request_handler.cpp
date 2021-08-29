#include "request_handler.h"

#ifdef MY_DEBUG
#include <fstream>
#endif
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
using namespace request_handler;
domain::BusStats RequestHandler::GetBusStats(const std::string &bus_name) const
{
    return db_.GetBusStats(bus_name);
}

// Возвращает маршруты, проходящие через
domain::StopStats RequestHandler::GetStopStats(const std::string &stop_name) const
{
    return db_.GetStopStats(stop_name);
}

svg::Document RequestHandler::RenderMap() const
{
    return renderer_.Render(db_);
}

json::Document request_handler::ProcessStatRequests(const RequestHandler &handler, const std::vector<json_reader::StatRequest> &requests)
{
    auto responses = json::Array{};
    for (auto &request : requests)
    {
        switch (request.request_type)
        {
        case json_reader::StatRequestType::BUS_REQUEST:
            responses.push_back(json_reader::MakeBusStatsNode(handler.GetBusStats(request.name).bus_stats, request.id));
            break;
        case json_reader::StatRequestType::STOP_REQUEST:
            responses.push_back(json_reader::MakeStopStatsNode(handler.GetStopStats(request.name).stop_data, request.id));
            break;
        case ::json_reader::StatRequestType::MAP_REQUEST:
#ifdef MY_DEBUG
            std::ofstream svg_out("output.svg");
            auto doc = handler.RenderMap();
            doc.Render(svg_out);
#endif
            responses.push_back(json_reader::MakeMapNode(handler.RenderMap(), request.id));
            break;
        }
    }
    return json::Document{std::move(responses)};
}