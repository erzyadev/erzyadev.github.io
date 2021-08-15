#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
using namespace request_handler;
domain::BusStats RequestHandler::GetBusStats(std::string bus_name) const
{
    return db_.GetBusStats(bus_name);
}

// Возвращает маршруты, проходящие через
domain::StopStats RequestHandler::GetStopStats(std::string stop_name) const
{
    return db_.GetStopStats(stop_name);
}

svg::Document RequestHandler::RenderMap() const
{
    return renderer_.Render(db_);
}