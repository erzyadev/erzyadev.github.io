#include "json_reader.h"
#include "json_builder.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader
{
    using namespace std;
    using namespace transport_catalogue;
    //    using namespace map_renderer;

    namespace
    {
        svg::Color GetColorFromText(const json::Node &color_node)
        {
            if (color_node.IsNull())
                return std::monostate{};
            else
            {
                if (color_node.IsArray())
                {
                    auto &color_array = color_node.AsArray();
                    if (color_array.size() == 4)
                    {
                        svg::Rgba result;
                        result.red = color_array[0].AsInt();
                        result.green = color_array[1].AsInt();
                        result.blue = color_array[2].AsInt();
                        result.opacity = color_array[3].AsDouble();
                        return result;
                    }
                    else
                    {
                        svg::Rgb result;
                        result.red = color_array[0].AsInt();
                        result.green = color_array[1].AsInt();
                        result.blue = color_array[2].AsInt();
                        return result;
                    }
                }
                else //string color
                    return color_node.AsString();
            }
        }
    }

    map_renderer::RenderSettings ReadRenderSetting(json::Node render_settings_node)
    {

        auto as_map = render_settings_node.AsMap();
        map_renderer::RenderSettings settings;
        settings.width = as_map.at("width").AsDouble();
        settings.height = as_map.at("height").AsDouble();

        settings.padding = as_map.at("padding").AsDouble();

        settings.line_width = as_map.at("line_width").AsDouble();
        settings.stop_radius = as_map.at("stop_radius").AsDouble();

        settings.stop_label_font_size = as_map.at("stop_label_font_size").AsDouble();
        auto &stop_label_offset = as_map.at("stop_label_offset").AsArray();
        settings.stop_label_offset.x = stop_label_offset[0].AsDouble();
        settings.stop_label_offset.y = stop_label_offset[1].AsDouble();

        settings.bus_label_font_size = as_map.at("bus_label_font_size").AsDouble();
        auto &bus_label_offset = as_map.at("bus_label_offset").AsArray();
        settings.bus_label_offset.x = bus_label_offset[0].AsDouble();
        settings.bus_label_offset.y = bus_label_offset[1].AsDouble();

        settings.underlayer_color = GetColorFromText(as_map.at("underlayer_color"));
        settings.underlayer_width = as_map.at("underlayer_width").AsDouble();

        auto &color_nodes = as_map.at("color_palette").AsArray();
        for (auto &color_node : color_nodes)
        {
            settings.color_palette.emplace_back(GetColorFromText(color_node));
        }
        return settings;
    }
    namespace
    {
        domain::Bus ReadBusFromJsonMap(const json::Dict &bus_map)
        {
            domain::Bus new_bus{bus_map.at("name").AsString()};
            for (auto &stop_node : bus_map.at("stops").AsArray())
            {
                new_bus.stops.push_back(stop_node.AsString());
            }
            new_bus.isLoop = bus_map.at("is_roundtrip").AsBool();
            return new_bus;
        }
        domain::Stop ReadStopFromJsonMap(const json::Dict &stop_map)
        {
            domain::Stop new_stop;
            new_stop.stop_name = stop_map.at("name").AsString();
            new_stop.coordinates.lat = stop_map.at("latitude").AsDouble();
            new_stop.coordinates.lng = stop_map.at("longitude").AsDouble();
            for (auto &[stop_name, distance_node] : stop_map.at("road_distances").AsMap())
            {
                new_stop.distances[stop_name] = distance_node.AsInt();
            }
            return new_stop;
        }
        json_reader::StatRequest ReadStatRequestFromJsonMap(const json::Dict &request_map)
        {
            StatRequest new_stat_request;
            new_stat_request.id = request_map.at("id").AsInt();
            if (request_map.at("type").AsString() == "Map"sv)
            {
                new_stat_request.request_type = StatRequestType::MAP_REQUEST;
            }
            else
            {
                new_stat_request.request_type = request_map.at("type").AsString() == "Bus" ? StatRequestType::BUS_REQUEST : StatRequestType::STOP_REQUEST;
                new_stat_request.name = request_map.at("name").AsString();
            }
            return new_stat_request;
        }
    }
    JsonReadResult ReadJson(std::istream &input)
    {
        JsonReadResult result;
        auto json_document = json::Load(input);
        auto &input_data_requests = json_document.GetRoot().AsMap().at("base_requests");
        auto &stat_request_data = json_document.GetRoot().AsMap().at("stat_requests");
        for (auto &request : input_data_requests.AsArray())
        {
            auto &as_map = request.AsMap();
            if (as_map.at("type").AsString() == "Bus")
            {
                result.new_buses.emplace_back(ReadBusFromJsonMap(as_map));
            }
            else if (as_map.at("type").AsString() == "Stop")
            {
                result.new_stops.emplace_back(ReadStopFromJsonMap(as_map));
            }
            else
                throw std::invalid_argument{"Invalid request type"};
        }
        for (auto &request : stat_request_data.AsArray())
        {
            result.stat_requests.emplace_back(ReadStatRequestFromJsonMap(request.AsMap()));
        }
        result.render_settings = ReadRenderSetting(json_document.GetRoot().AsMap().at("render_settings"));
        return result;
    }

    json::Node MakeBusStatsNode(const std::optional<domain::BusData> &bus_data, int request_id)
    {

        using namespace json;
        auto result_builder = Builder{}.StartDict().Key("request_id").Value(request_id);
        if (bus_data)
        {
            return result_builder
                .Key("curvature")
                .Value(bus_data->curvature)

                .Key("route_length")
                .Value(bus_data->route_length)

                .Key("unique_stop_count")
                .Value(bus_data->unique_stops)

                .Key("stop_count")
                .Value(bus_data->total_stops)

                .EndDict()
                .Build();
        }
        else
        {
            return result_builder
                .Key("error_message")
                .Value("not found"s)
                .EndDict()
                .Build();
        }
    }

    json::Node MakeStopStatsNode(const std::optional<domain::StopData> &stop_data, int request_id)
    {

        auto result_builder = json::Builder()
                                  .StartDict()
                                  .Key("request_id")
                                  .Value(request_id);
        if (!stop_data)
            return result_builder.Key("error_message").Value("not found"s).EndDict().Build();

        auto array_node_builder = json::Builder{}.StartArray();
        for (auto &bus_number : stop_data->buses)
            array_node_builder.Value(bus_number);

        return result_builder.Key("buses").Value(array_node_builder.EndArray().Build().AsArray()).EndDict().Build();
    }

    json::Node MakeMapNode(svg::Document rendered_map, int request_id)
    {

        ostringstream map_output;
        rendered_map.Render(map_output);

        return json::Builder{}
            .StartDict()

            .Key("request_id")
            .Value(request_id)

            .Key("map")
            .Value(map_output.str())

            .EndDict()
            .Build();
    }

}