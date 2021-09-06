#include "transport_router.h"

namespace transport_router
{
    using namespace std;
    using graph::DirectedWeightedGraph, graph::Edge;
    graph::DirectedWeightedGraph<double> TransportRouter::BuildGraph()
    {
        unordered_map<string, size_t> stop_index;
        auto &stops = catalogue_.GetStops();
        auto &buses = catalogue_.GetBuses();
        for (size_t i = 0; i < catalogue_.GetStops().size(); ++i)
        {
            stop_index[stops[i].stop_name] = i;
        }
        graph::DirectedWeightedGraph<double> stop_graph(catalogue_.GetStops().size());
        for (auto &bus : buses)
        {
            for (auto stop_it = bus.stops.begin(); stop_it != bus.stops.end(); ++stop_it)
            {
                for (auto to_it = next(stop_it); to_it != bus.stops.end(); ++to_it)
                {
                    // stop_graph.AddEdge(Edge<double>{
                    //     stop_index[*stop_it],stop_index[*to_it], catalogue_.
                    // })
                }
            }
        }
    }
}