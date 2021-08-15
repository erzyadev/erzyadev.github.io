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

svg::Document map_renderer::MapRenderer::Render(const transport_catalogue::TransportCatalogue &catalogue) const
{
    //throw std::runtime_error{"Not implemented"};
    //return svg::Document{};
    using namespace svg;
    auto map_data = catalogue.GetMapData();
    Document rendered_map;
    std::sort(map_data.begin(), map_data.end(), [](auto &bus1, auto &bus2)
              { return bus1.bus_number < bus2.bus_number; });

    auto stop_coordinates = catalogue.GetStopCoordinates();
    SphereProjector projector{stop_coordinates.begin(), stop_coordinates.end(),
                              settings_.width, settings_.height, settings_.padding};

    int palette_size = settings_.color_palette.size();
    for (int i = 0; i < map_data.size(); ++i)
    {
        auto &bus = map_data[i];
        Polyline route;
        for (auto &stop : bus.stops)
        {
            route.AddPoint(projector(stop.coordinates));
        }
        route.SetFillColor(settings_.color_palette[i % palette_size]);
        rendered_map.Add(route);
    }

    //Draw stop circles
    //Draw stop names
    //Something else
    return rendered_map;
}