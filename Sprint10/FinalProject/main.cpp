// int main() {
//     /*
//      * Примерная структура программы:
//      *
//      * Считать JSON из stdin
//      * Построить на его основе JSON базу данных транспортного справочника
//      * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
//      * с ответами Вывести в stdout ответы в виде JSON
//      */
// }

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <fstream>
#include <iostream>

using namespace std;
int main()
{
#ifdef MY_DEBUG
    ifstream in("../s10_final_opentest/s10_final_opentest_1.json");
#else
    auto &in = cin;
#endif
    using namespace transport_catalogue;
    cout.precision(6);
    auto json_parse_result = json_reader::ReadJson(in);
    auto catalogue = TransportCatalogue{std::move(json_parse_result.new_stops), std::move(json_parse_result.new_buses)};
    auto render_settings = json_parse_result.render_settings;
    auto responses = json::Array{};
    auto renderer = map_renderer::MapRenderer{render_settings};
    auto handler = request_handler::RequestHandler(catalogue, renderer);
    for (auto &request : json_parse_result.stat_requests)
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
            responses.push_back(json_reader::MakeMapNode(handler.RenderMap(), request.id));
            break;
        }
    }
    auto result_document = json::Document(json::Node(responses));
    json::Print(result_document, cout);
}