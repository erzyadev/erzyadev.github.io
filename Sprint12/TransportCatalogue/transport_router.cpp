#include "transport_router.h"

namespace transport_router
{
    using namespace std;
    using graph::DirectedWeightedGraph, graph::Edge;

    void TransportRouter::InitStopIdIndex()
    {
        auto &stops = catalogue_.GetStops();
        for (size_t i = 0; i < stops.size(); ++i)
        {
            stop_to_id_[stops[i].stop_name] = i;
            id_to_stop_[i] = stops[i].stop_name;
        }
    }
    graph::DirectedWeightedGraph<double> TransportRouter::BuildGraphInitIndex()
    {
        InitStopIdIndex();
        graph::DirectedWeightedGraph<double> stop_graph(catalogue_.GetStops().size());
        for (auto &bus : catalogue_.GetBuses())
        {
            for (auto from_it = bus.stops.begin(); from_it != prev(bus.stops.end()); ++from_it)
            {
                double distance = 0;
                size_t span = 0;
                for (auto to_it = next(from_it); to_it != bus.stops.end(); ++to_it)
                {
                    distance += catalogue_.GetDistance(*prev(to_it), *to_it);
                    span += 1;

                    size_t from_id = stop_to_id_[*from_it];
                    size_t to_id = stop_to_id_[*to_it];
                    stop_graph.AddEdge(EdgeType{
                        from_id,
                        to_id,
                        static_cast<double>(routing_settings_.bus_wait_time) + distance / routing_settings_.bus_velocity,
                        span,
                        bus.bus_number});
                }
            }
        }
        return stop_graph;
    }

    std::optional<Route> TransportRouter::GetRoute(const std::string &from, const std::string &to) const
    {
        auto raw_route = GetRawRoute(from, to);
        if (!raw_route)
            return {};
        std::vector<std::unique_ptr<RouteItem>> route_items = {};

        for (auto &edgeId : raw_route->edges)
        {

            auto &edge = stop_graph_.GetEdge(edgeId);
            const std::string &from_name = id_to_stop_.at(edge.from);
            route_items.push_back(std::make_unique<WaitItem>(routing_settings_.bus_wait_time, from_name));

            route_items.push_back(std::make_unique<RideItem>(edge.weight - routing_settings_.bus_wait_time,
                                                             edge.bus_number,
                                                             edge.span));
        }
        return Route{raw_route->weight, move(route_items)};
    }
}