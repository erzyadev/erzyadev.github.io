

#include "request_handler.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include <fstream>
#include <iostream>

using namespace std;

int main()
{
#ifdef MY_DEBUG
    ifstream in("../input2.json");
#else
    auto &in = cin;
#endif
    using namespace transport_catalogue;
    auto json_parse_result = json_reader::ReadJson(in);
    auto catalogue = TransportCatalogue{std::move(json_parse_result.new_stops), std::move(json_parse_result.new_buses)};
    auto render_settings = json_parse_result.render_settings;
    auto responses = json::Array{};
    auto renderer = map_renderer::MapRenderer{render_settings};
    auto handler = request_handler::RequestHandler(catalogue, renderer);

    auto result_document = request_handler::ProcessStatRequests(handler,
                                                                json_parse_result.stat_requests);
    json::Print(result_document, cout);
}
