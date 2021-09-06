#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"

namespace transport_router
{
    struct RoutingSettings
    {
        int bus_wait_time;
        double bus_velocity;
    };

    class TransportRouter
    {
        TransportRouter(const transport_catalogue::TransportCatalogue &catalogue, RoutingSettings settings)
            : catalogue_{catalogue}, routing_settings_{settings}
        {
        }

    private:
        const transport_catalogue::TransportCatalogue &catalogue_;
        RoutingSettings routing_settings_;
        graph::DirectedWeightedGraph<double> BuildGraph();
    };
}