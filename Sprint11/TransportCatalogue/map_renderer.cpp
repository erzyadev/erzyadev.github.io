#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

svg::Point map_renderer::SphereProjector::operator()(geo::Coordinates coords) const
{
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

bool map_renderer::IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

namespace
{
    template <typename BusIter>
    auto populate_ptrs(
        BusIter item_begin, BusIter item_end)
    {
        std::vector<const typename BusIter::value_type *> result;
        for (auto iter = item_begin; iter != item_end; ++iter)
        {
            result.push_back(&*iter);
        }
        return result;
    }
}

svg::Document map_renderer::MapRenderer::Render(const transport_catalogue::TransportCatalogue &catalogue) const
{
    //throw std::runtime_error{"Not implemented"};
    //return svg::Document{};
    using namespace svg;
    //auto map_data = catalogue.GetMapData();
    Document rendered_map;
    // std::sort(map_data.begin(), map_data.end(), [](auto &bus1, auto &bus2)
    //           { return bus1.bus_number < bus2.bus_number; });

    auto stop_coordinates = catalogue.GetNonemptyStopCoordinates();
    SphereProjector projector{stop_coordinates.begin(), stop_coordinates.end(),
                              settings_.width, settings_.height, settings_.padding};

    int palette_size = settings_.color_palette.size();
    auto &buses = catalogue.GetBuses();
    auto &stops = catalogue.GetStops();
    auto bus_ptrs = populate_ptrs(buses.cbegin(), buses.cend());
    std::sort(bus_ptrs.begin(), bus_ptrs.end(),
              [](const domain::Bus *x, const domain::Bus *y)
              { return x->bus_number < y->bus_number; });

    int color_counter = 0;
    for (auto bus_ptr : bus_ptrs)
    {
        auto &bus = *bus_ptr;
        if (!bus.stops.empty())
        {
            Polyline route;
            for (auto &stop : bus.stops)
            {
                route.AddPoint(projector(catalogue.GetStopCoordinates(stop)));
            }
            if (!bus.isLoop)
            {
                for (auto reverse_stop_it = next(bus.stops.rbegin());
                     reverse_stop_it != bus.stops.rend(); ++reverse_stop_it)
                {
                    route.AddPoint(projector(catalogue.GetStopCoordinates(*reverse_stop_it)));
                }
            }
            svg::Color current_route_color = settings_.color_palette[color_counter];
            route.SetStrokeColor(current_route_color);
            route.SetFillColor(svg::NoneColor);
            route.SetStrokeWidth(settings_.line_width);
            route.SetStrokeLineCap(StrokeLineCap::ROUND);
            route.SetStrokeLineJoin(StrokeLineJoin::ROUND);
            rendered_map.Add(route);
            color_counter = (color_counter + 1) % palette_size;
        }
    }

    color_counter = 0;
    for (auto bus_ptr : bus_ptrs)
    {
        auto &bus = *bus_ptr;
        if (!bus.stops.empty())
        {
            //Draw stop names
            svg::Color current_route_color = settings_.color_palette[color_counter];

            Text route_name;
            route_name.SetData(bus.bus_number);
            route_name.SetPosition(projector(catalogue.GetStopCoordinates(bus.stops.front())));
            route_name.SetOffset(settings_.bus_label_offset);
            route_name.SetFontSize(settings_.bus_label_font_size);
            route_name.SetFontFamily("Verdana");
            route_name.SetFontWeight("bold");
            route_name.SetFillColor(current_route_color);

            Text route_name_underlayer = route_name;
            route_name_underlayer.SetFillColor(settings_.underlayer_color);
            route_name_underlayer.SetStrokeColor(settings_.underlayer_color);
            route_name_underlayer.SetStrokeWidth(settings_.underlayer_width);
            route_name_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            route_name_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            rendered_map.Add(route_name_underlayer);
            rendered_map.Add(route_name);
            if (!bus.isLoop && bus.stops.back() != bus.stops.front())
            {
                Text last_stop_underlayer{route_name_underlayer};
                last_stop_underlayer.SetPosition(projector(catalogue.GetStopCoordinates(bus.stops.back())));
                rendered_map.Add(last_stop_underlayer);
                Text last_stop{route_name};
                last_stop.SetPosition(projector(catalogue.GetStopCoordinates(bus.stops.back())));
                rendered_map.Add(last_stop);
            }

            color_counter = (color_counter + 1) % palette_size;
        }
    }

    auto stop_ptrs = populate_ptrs(stops.begin(), stops.end());
    std::sort(stop_ptrs.begin(), stop_ptrs.end(), [](const domain::Stop *x, const domain::Stop *y)
              { return x->stop_name < y->stop_name; });
    for (auto stop_ptr : stop_ptrs)
    {
        if (!stop_ptr->buses.empty())
        {
            svg::Circle stop_circle;
            stop_circle.SetCenter(projector(stop_ptr->coordinates));
            stop_circle.SetRadius(settings_.stop_radius);
            stop_circle.SetFillColor("white");
            rendered_map.Add(stop_circle);
        }
    }

    for (auto stop_ptr : stop_ptrs)
    {
        if (!stop_ptr->buses.empty())
        {
            Text stop_name;
            stop_name.SetPosition(projector(stop_ptr->coordinates));
            stop_name.SetOffset(settings_.stop_label_offset);
            stop_name.SetFontSize(settings_.stop_label_font_size);
            stop_name.SetFontFamily("Verdana");
            stop_name.SetData(stop_ptr->stop_name);

            Text stop_name_underlayer{stop_name};
            stop_name_underlayer.SetFillColor(settings_.underlayer_color);
            stop_name_underlayer.SetStrokeColor(settings_.underlayer_color);
            stop_name_underlayer.SetStrokeWidth(settings_.underlayer_width);
            stop_name_underlayer.SetStrokeLineCap(StrokeLineCap::ROUND);
            stop_name_underlayer.SetStrokeLineJoin(StrokeLineJoin::ROUND);
            rendered_map.Add(stop_name_underlayer);

            stop_name.SetFillColor("black");
            rendered_map.Add(stop_name);
        }
    }

    //Draw stop circles
    //Draw stop names
    //Something else
    return rendered_map;
}