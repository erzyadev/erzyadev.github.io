#pragma once
#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include <optional>
#include <iostream>
#include "geo.h"
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <unordered_set>

namespace transport_catalogue
{
    struct DistanceTo
    {
        std::string_view to_stop;
        int distance;
    };
    struct Bus
    {
        Bus(std::string_view new_bus_number) : bus_number{new_bus_number} {}

        const std::string bus_number;
        std::vector<std::string> stops;
        bool isLoop;
    };

    struct NewStop
    {
        std::string_view stop_name;
        geo::Coordinates coordinates;
        std::vector<std::string_view> buses;
        std::vector<DistanceTo> distances;
    };

    struct Stop
    {
        Stop(const NewStop &new_stop) : stop_name{new_stop.stop_name},
                                        coordinates(new_stop.coordinates) {}

        const std::string stop_name;
        geo::Coordinates coordinates;
        std::unordered_set<std::string_view> buses;
        std::unordered_map<std::string_view, double> distances;
    };

    struct BusData
    {
        int total_stops;
        int unique_stops;
        double route_length;
        double curvature;
    };

    struct BusStats
    {
        std::string_view bus_number;
        std::optional<BusData> bus_stats;
    };

    struct StopData
    {
        const std::vector<std::string_view> buses;
    };

    struct StopStats
    {
        std::string_view stop_name;
        std::optional<StopData> stop_data;
    };

    class TransportCatalogue
    {
    public:
        TransportCatalogue(std::vector<NewStop> &&new_stops, std::vector<Bus> &&new_buses);

        BusStats GetBusStats(std::string_view bus_number) const;
        StopStats GetStopStats(std::string_view stop_name) const;

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;

        std::unordered_map<std::string_view, Bus *> bus_index_;
        std::unordered_map<std::string_view, Stop *> stop_index_;

        std::unordered_map<std::string_view, BusData> bus_statistics_;
        std::unordered_map<std::string_view, StopData> stop_statistics_;

        BusData CalculateBusStats(const Bus &bus) const;
        StopData CalculateStopStats(const Stop &stop_data) const;

        double GetGeographicalDistance(std::string_view from, std::string_view to) const;
        double GetDistanceStopsLoop(std::string_view from, std::string_view to) const;
        double GetDistanceStopsReturn(std::string_view from, std::string_view to) const;
    };

    std::ostream &operator<<(std::ostream &out, const BusStats &busInfo);

    std::ostream &operator<<(std::ostream &out, const StopStats &stopInfo);
}