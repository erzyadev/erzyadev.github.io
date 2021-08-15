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
#include "domain.h"
#include <map>

namespace transport_catalogue
{
    using namespace domain;

    struct StopMapInfo
    {
        std::string stop_name;
        geo::Coordinates coordinates;
    };
    struct BusMapInfo
    {
        std::string bus_number;
        std::vector<StopMapInfo> stops;
    };

    class TransportCatalogue
    {
    public:
        TransportCatalogue(std::vector<Stop> &&new_stops, std::vector<Bus> &&new_buses);

        BusStats GetBusStats(std::string bus_number) const;
        StopStats GetStopStats(std::string stop_name) const;

        auto &GetBuses() const
        {
            return buses_;
        }

        auto &GetStops() const
        {
            return stops_;
        }

        std::vector<BusMapInfo> GetMapData() const
        {
            std::vector<BusMapInfo> map_data;
            for (auto &bus : buses_)
            {
                auto &new_bus = map_data.emplace_back();
                new_bus.bus_number = bus.bus_number;
                for (auto &stop : bus.stops)
                {
                    new_bus.stops.push_back({stop, stop_index_.at(stop)->coordinates});
                }
                if (!bus.isLoop)
                {
                    for (auto reverse_it = next(bus.stops.rbegin());
                         reverse_it != bus.stops.rend();
                         ++reverse_it)
                    {
                        new_bus.stops.push_back({*reverse_it,
                                                 stop_index_.at(*reverse_it)->coordinates});
                    }
                }
            }
            return map_data;
        }

        std::vector<geo::Coordinates> GetStopCoordinates() const
        {
            std::vector<geo::Coordinates> stop_coordinates;
            std::transform(stops_.begin(), stops_.end(), std::back_inserter(stop_coordinates), [](const Stop &stop)
                           { return stop.coordinates; });
            return stop_coordinates;
        }
        geo::Coordinates GetStopCoordinates(std::string stop_name) const
        {
            return stop_index_.at(stop_name)->coordinates;
        }

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;

        std::unordered_map<std::string, Bus *> bus_index_;
        std::unordered_map<std::string, Stop *> stop_index_;

        std::unordered_map<std::string, BusData> bus_statistics_;
        std::unordered_map<std::string, StopData> stop_statistics_;

        BusData CalculateBusData(const Bus &bus) const;
        StopData CalculateStopData(const Stop &stop_data) const;

        double GetGeographicalDistance(std::string from, std::string to) const;
        double GetDistanceStopsLoop(std::string from, std::string to) const;
        double GetDistanceStopsReturn(std::string from, std::string to) const;
    };

    std::ostream &operator<<(std::ostream &out, const BusStats &busInfo);

    std::ostream &operator<<(std::ostream &out, const StopStats &stopInfo);
}