#include "map_renderer.h"

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

namespace
{
    template <typename Container, typename PtrCmp>
    std::vector<const typename Container::value_type *> GetSortedElementPointers(const Container &container, PtrCmp ptr_cmp)
    {
        std::vector<const typename Container::value_type *> ptrs = populate_ptrs(container.begin(), container.end());
        std::sort(ptrs.begin(), ptrs.end(), ptr_cmp);
        return ptrs;
    }

    void DrawStopCircle(svg::Document &rendered_map, svg::Point position, const map_renderer::RenderSettings &settings)
    {
        svg::Circle stop_circle;
        stop_circle.SetCenter(position);
        stop_circle.SetRadius(settings.stop_radius);
        stop_circle.SetFillColor("white");
        rendered_map.Add(stop_circle);
    }

    void DrawStopName(svg::Document &rendered_map, const std::string &stop_name, svg::Point position, const map_renderer::RenderSettings &settings)
    {
        using namespace svg;
        svg::Text stop_name_svg;
        stop_name_svg.SetPosition(position);
        stop_name_svg.SetOffset(settings.stop_label_offset);
        stop_name_svg.SetFontSize(settings.stop_label_font_size);
        stop_name_svg.SetFontFamily("Verdana");
        stop_name_svg.SetData(stop_name);

        svg::Text stop_name_underlayer{stop_name_svg};
        stop_name_underlayer.SetFillColor(settings.underlayer_color);
        stop_name_underlayer.SetStrokeColor(settings.underlayer_color);
        stop_name_underlayer.SetStrokeWidth(settings.underlayer_width);
        stop_name_underlayer.SetStrokeLineCap(StrokeLineCap::ROUND);
        stop_name_underlayer.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        rendered_map.Add(stop_name_underlayer);

        stop_name_svg.SetFillColor("black");
        rendered_map.Add(stop_name_svg);
    }
    svg::Text SetUpRouteName(const std::string &bus_number, svg::Point first_stop_position, const svg::Color &route_color,
                             const map_renderer::RenderSettings &settings)
    {
        svg::Text route_name;
        route_name.SetData(bus_number);
        route_name.SetPosition(first_stop_position);
        route_name.SetOffset(settings.bus_label_offset);
        route_name.SetFontSize(settings.bus_label_font_size);
        route_name.SetFontFamily("Verdana");
        route_name.SetFontWeight("bold");
        route_name.SetFillColor(route_color);
        return route_name;
    }
    svg::Text SetUpRouteNameUnderlayer(const svg::Text &route_name, const map_renderer::RenderSettings &settings)
    {
        svg::Text route_name_underlayer = route_name;
        route_name_underlayer.SetFillColor(settings.underlayer_color);
        route_name_underlayer.SetStrokeColor(settings.underlayer_color);
        route_name_underlayer.SetStrokeWidth(settings.underlayer_width);
        route_name_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        route_name_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        return route_name_underlayer;
    }

    std::pair<svg::Text, svg::Text> RenderRouteName(svg::Document &rendered_map, const std::string &bus_number, svg::Point first_stop_position, const svg::Color &route_color,
                                                    const map_renderer::RenderSettings &settings)
    {
        svg::Text route_name = SetUpRouteName(bus_number, first_stop_position,
                                              route_color, settings);
        svg::Text route_name_underlayer = SetUpRouteNameUnderlayer(route_name, settings);
        rendered_map.Add(route_name_underlayer);
        rendered_map.Add(route_name);
        return {route_name, route_name_underlayer};
    }

    void RenderRouteNameLastStop(svg::Document &rendered_map, const svg::Text &route_name,
                                 const svg::Text &route_name_underlayer, svg::Point position)
    {
        svg::Text last_stop_underlayer{route_name_underlayer};
        last_stop_underlayer.SetPosition(position);
        rendered_map.Add(last_stop_underlayer);
        svg::Text last_stop{route_name};
        last_stop.SetPosition(position);
        rendered_map.Add(last_stop);
    }

    template <typename CoordMapper>
    void DrawRoute(svg::Document &rendered_map,
                   const domain::Bus &bus, const CoordMapper &coordinate_to_position, svg::Color route_color,
                   const map_renderer::RenderSettings &settings)
    {
        using namespace svg;
        Polyline route;
        for (auto &stop : bus.stops)
        {
            route.AddPoint(coordinate_to_position(stop));
        }
        // if (!bus.isLoop)
        // {
        //     for (auto reverse_stop_it = next(bus.stops.rbegin());
        //          reverse_stop_it != bus.stops.rend(); ++reverse_stop_it)
        //     {
        //         route.AddPoint(coordinate_to_position(*reverse_stop_it));
        //     }
        // }
        //svg::Color current_route_color = settings_.color_palette[color_counter];
        route.SetStrokeColor(route_color);
        route.SetFillColor(svg::NoneColor);
        route.SetStrokeWidth(settings.line_width);
        route.SetStrokeLineCap(StrokeLineCap::ROUND);
        route.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        rendered_map.Add(route);
    }

    map_renderer::SphereProjector MakeProjector(const transport_catalogue::TransportCatalogue &catalogue,
                                                const map_renderer::RenderSettings &settings)
    {
        auto stop_coordinates = catalogue.GetNonemptyStopCoordinates();
        return {stop_coordinates.begin(), stop_coordinates.end(),
                settings.width, settings.height, settings.padding};
    }

    template <typename CoordMapper>
    void DrawRoutes(svg::Document &rendered_map,
                    class std::vector<const domain::Bus *> bus_ptrs, CoordMapper coordinate_to_position,
                    const map_renderer::RenderSettings &settings)
    {
        size_t palette_size = settings.color_palette.size();
        size_t color_counter = 0;
        for (auto bus_ptr : bus_ptrs)
        {
            auto &bus = *bus_ptr;
            if (!bus.stops.empty())
            {
                svg::Color current_route_color = settings.color_palette[color_counter];

                DrawRoute(
                    rendered_map,
                    bus,
                    coordinate_to_position,
                    current_route_color, settings);
                color_counter = (color_counter + 1) % palette_size;
            }
        }
    }

    template <typename CoordMapper>
    void DrawRouteNames(svg::Document &rendered_map, class std::vector<const domain::Bus *> bus_ptrs, CoordMapper coordinate_to_position,
                        const map_renderer::RenderSettings &settings)
    {
        size_t palette_size = settings.color_palette.size();
        size_t color_counter = 0;
        for (auto bus_ptr : bus_ptrs)
        {
            auto &bus = *bus_ptr;
            if (!bus.stops.empty())
            {

                svg::Color current_route_color = settings.color_palette[color_counter];

                const auto &[route_name, route_name_underlayer] = RenderRouteName(rendered_map, bus.bus_number,
                                                                                  coordinate_to_position(bus.stops.front()),
                                                                                  current_route_color, settings);
                if (!bus.isLoop && bus.stops[bus.stops.size() / 2] != bus.stops.front())
                {

                    RenderRouteNameLastStop(rendered_map, route_name, route_name_underlayer,
                                            coordinate_to_position(bus.stops[bus.stops.size() / 2]));
                }

                color_counter = (color_counter + 1) % palette_size;
            }
        }
    }

    void DrawStopCircles(svg::Document &rendered_map,
                         std::vector<const domain::Stop *> stop_ptrs,
                         const map_renderer::SphereProjector &projector,
                         const map_renderer::RenderSettings &settings)
    {

        for (auto stop_ptr : stop_ptrs)
        {
            if (!stop_ptr->buses.empty())
            {
                DrawStopCircle(rendered_map, projector(stop_ptr->coordinates), settings);
            }
        }
    }

    void DrawStopNames(svg::Document &rendered_map,
                       std::vector<const domain::Stop *> stop_ptrs,
                       const map_renderer::SphereProjector &projector,
                       const map_renderer::RenderSettings &settings)
    {
        for (auto stop_ptr : stop_ptrs)
        {
            if (!stop_ptr->buses.empty())
            {
                DrawStopName(rendered_map, stop_ptr->stop_name, projector(stop_ptr->coordinates), settings);
            }
        }
    }
}

svg::Document map_renderer::MapRenderer::Render(const transport_catalogue::TransportCatalogue &catalogue) const
{

    using namespace svg;

    Document rendered_map;

    SphereProjector projector = MakeProjector(catalogue, settings_);

    auto &buses = catalogue.GetBuses();
    auto bus_ptrs = GetSortedElementPointers(buses, [](const domain::Bus *x, const domain::Bus *y)
                                             { return x->bus_number < y->bus_number; });

    auto coordinate_to_position = [&projector, &catalogue](const std::string &stop)
    { return projector(catalogue.GetStopCoordinates(stop)); };

    DrawRoutes(rendered_map, bus_ptrs, coordinate_to_position,
               settings_);
    DrawRouteNames(rendered_map, bus_ptrs, coordinate_to_position, settings_);

    auto &stops = catalogue.GetStops();
    auto stop_ptrs = GetSortedElementPointers(stops, [](const domain::Stop *x, const domain::Stop *y)
                                              { return x->stop_name < y->stop_name; });

    DrawStopCircles(rendered_map, stop_ptrs, projector, settings_);

    DrawStopNames(rendered_map, stop_ptrs, projector, settings_);
    return rendered_map;
}