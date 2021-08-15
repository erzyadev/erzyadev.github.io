#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader
{
    using namespace std;
    using namespace transport_catalogue;
    using namespace map_renderer;

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

    RenderSettings ReadRenderSetting(json::Node render_settings_node)
    {

        auto as_map = render_settings_node.AsMap();
        RenderSettings settings;
        settings.width = as_map.at("width").AsDouble();
        settings.height = as_map.at("height").AsDouble();

        settings.padding = as_map.at("padding").AsDouble();

        settings.line_width = as_map.at("line_width").AsDouble();
        settings.stop_radius = as_map.at("stop_radius").AsDouble();

        settings.stop_label_font_size = as_map.at("stop_label_font_size").AsDouble();
        for (auto &double_node : as_map.at("stop_label_offset").AsArray())
        {
            settings.stop_label_offset.push_back(double_node.AsDouble());
        }

        settings.bus_label_font_size = as_map.at("bus_label_font_size").AsDouble();
        for (auto &double_node : as_map.at("bus_label_offset").AsArray())
        {
            settings.bus_label_offset.push_back(double_node.AsDouble());
        }

        settings.underlayer_color = GetColorFromText(as_map.at("underlayer_color"));
        settings.underlayer_width = as_map.at("underlayer_width").AsDouble();

        auto &color_nodes = as_map.at("color_palette").AsArray();
        for (auto &color_node : color_nodes)
        {
            settings.color_palette.emplace_back(GetColorFromText(color_node));
        }
        return settings;
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
                auto &new_bus = result.new_buses.emplace_back(as_map.at("name").AsString());
                for (auto &stop_node : as_map.at("stops").AsArray())
                {
                    new_bus.stops.push_back(stop_node.AsString());
                }
                new_bus.isLoop = as_map.at("is_roundtrip").AsBool();
            }
            else if (as_map.at("type").AsString() == "Stop")
            {
                auto &new_stop = result.new_stops.emplace_back();
                new_stop.stop_name = as_map.at("name").AsString();
                new_stop.coordinates.lat = as_map.at("latitude").AsDouble();
                new_stop.coordinates.lng = as_map.at("longitude").AsDouble();
                for (auto &[stop_name, distance_node] : as_map.at("road_distances").AsMap())
                {
                    new_stop.distances[stop_name] = distance_node.AsInt();
                }
            }
            else
                throw std::invalid_argument{"Invalid request type"};
        }
        for (auto &request : stat_request_data.AsArray())
        {
            auto &as_map = request.AsMap();
            auto &new_stat_request = result.stat_requests.emplace_back();
            new_stat_request.id = as_map.at("id").AsInt();
            if (as_map.at("type").AsString() == "Map"sv)
            {
                new_stat_request.request_type = StatRequestType::MAP_REQUEST;
            }
            else
            {
                new_stat_request.request_type = as_map.at("type").AsString() == "Bus" ? StatRequestType::BUS_REQUEST : StatRequestType::STOP_REQUEST;
                new_stat_request.name = as_map.at("name").AsString();
            }
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