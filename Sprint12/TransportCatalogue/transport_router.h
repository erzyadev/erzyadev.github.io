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

    struct RideItem;
    struct WaitItem;

    struct RouteItemVisitor
    {
        virtual void Visit(RideItem &item) = 0;
        virtual void Visit(WaitItem &item) = 0;
        virtual ~RouteItemVisitor() = default;
    };
    struct RouteItem
    {
        RouteItem(double time) : time(time) {}
        double time;
        virtual ~RouteItem() = default;
        virtual void Accept(RouteItemVisitor &) = 0;
    };

    template <typename ItemType>
    struct RouteItemVisitable : RouteItem
    {
        using RouteItem::RouteItem;
        void Accept(RouteItemVisitor &visitor) override
        {
            visitor.Visit(static_cast<ItemType &>(*this));
        }
    };

    struct WaitItem : public RouteItemVisitable<WaitItem>
    {
        WaitItem(double time, std::string stop_name) : RouteItemVisitable<WaitItem>(time), stop_name(move(stop_name)) {}
        std::string stop_name;
    };

    struct RideItem : public RouteItemVisitable<RideItem>
    {
        RideItem(double time, std::string bus_number, size_t span_count)
            : RouteItemVisitable<RideItem>(time), bus_number(move(bus_number)), span_count(span_count) {}
        std::string bus_number;
        size_t span_count;
    };

    struct Route
    {
        double total_time;
        std::vector<std::unique_ptr<RouteItem>> items;
    };

    class TransportRouter
    {
    public:
        TransportRouter(const transport_catalogue::TransportCatalogue &catalogue, RoutingSettings settings)
            : catalogue_{catalogue}, routing_settings_{settings}, stop_graph_{BuildGraph()}, router_(stop_graph_)
        {
        }

        std::optional<Route> GetRoute(const std::string &from, const std::string &to) const;

    private:
        using RouterType = graph::Router<double>;
        using GraphType = graph::DirectedWeightedGraph<double>;
        const transport_catalogue::TransportCatalogue &catalogue_;
        RoutingSettings routing_settings_;
        graph::DirectedWeightedGraph<double> BuildGraph();
        double GetTravelTime(double distance)
        {
            return distance / routing_settings_.bus_velocity;
        }
        std::unordered_map<std::string, size_t> stop_to_id_;
        std::unordered_map<size_t, std::string> id_to_stop_;
        GraphType stop_graph_;
        RouterType router_;

        std::optional<graph::Router<double>::RouteInfo> GetRawRoute(const std::string &from, const std::string &to) const
        {
            size_t from_id = stop_to_id_.at(from);
            size_t to_id = stop_to_id_.at(to);
            return router_.BuildRoute(from_id, to_id);
        }
    };
}