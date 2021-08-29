#include "json_reader.h"

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
        json::Dict result;
        result["request_id"] = json::Node(request_id);
        if (bus_data)
        {
            result["curvature"] = json::Node(bus_data->curvature);
            result["route_length"] = json::Node(bus_data->route_length);
            result["unique_stop_count"] = json::Node(bus_data->unique_stops);
            result["stop_count"] = json::Node(bus_data->total_stops);
        }
        else
        {
            result["error_message"] = json::Node("not found"s);
        }
        return result;
    }

    json::Node MakeStopStatsNode(const std::optional<domain::StopData> &stop_data, int request_id)
    {
        json::Dict result;
        result["request_id"] = json::Node(request_id);
        if (stop_data)
        {
            auto bus_node_array = json::Array{};
            if (!stop_data->buses.empty())
            {
                for (auto &bus_number : stop_data->buses)
                {
                    bus_node_array.push_back(json::Node(bus_number));
                }
                result["buses"] = bus_node_array;
            }
            else
            {
                result["buses"] = json::Array{};
            }
        }
        else
        {
            result["error_message"] = "not found"s;
        }
        return result;
    }

    json::Node MakeMapNode(svg::Document rendered_map, int request_id)
    {

        ostringstream map_output;
        rendered_map.Render(map_output);
        json::Dict result_dict;
        result_dict["request_id"] = json::Node(request_id);
        result_dict["map"] = json::Node(map_output.str());
        return json::Node(result_dict);
    }

}