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

// Возвращает маршруты, проходящие через остановку
domain::StopStats RequestHandler::GetStopStats(const std::string &stop_name) const
{
    return db_.GetStopStats(stop_name);
}

svg::Document RequestHandler::RenderMap() const
{
    return renderer_.Render(db_);
}

json::Document request_handler::ProcessStatRequests(const RequestHandler &handler, const std::vector<std::unique_ptr<json_reader::StatRequest>> &requests)
{
    auto responses = json::Array{};
    for (auto &request : requests)
    {
        responses.push_back(request->GetProcessed(handler));
    }
    return json::Document{std::move(responses)};
}