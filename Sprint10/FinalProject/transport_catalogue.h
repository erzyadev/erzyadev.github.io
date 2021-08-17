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

        std::vector<geo::Coordinates> GetNonemptyStopCoordinates() const;

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