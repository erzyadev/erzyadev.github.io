// напишите решение с нуля
// код сохраните в свой git-репозиторий

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
#include <unordered_set>
struct Bus
{
    int bus_number;
    std::vector<std::string> stops;
    bool isLoop;
};

struct Stop
{
    std::string stop_name;
    Coordinates coordinates;
};

struct BusStats
{
    int total_stops;
    int unique_stops;
    double route_length;
};

struct BusInfo
{
    int bus_number;
    std::optional<BusStats> bus_stats;
};

class TransportCatalogue
{
public:
    void AddStop(Stop stop)
    {
        stops_.push_back(stop);
        stop_index_[stops_.back().stop_name] = &stops_.back();
    }
    void AddBus(Bus bus)
    {
        buses_.push_back(bus);
        bus_index_[bus.bus_number] = &buses_.back();
    }

    BusInfo GetStats(int bus_number) const
    {

        BusInfo result{bus_number, std::nullopt};
        if (auto it = bus_index_.find(bus_number); it != bus_index_.end())
        {
            auto &bus_stats = result.bus_stats;
            bus_stats = BusStats{};
            auto bus_ptr = it->second;
            bus_stats->total_stops = bus_ptr->isLoop ? bus_ptr->stops.size() : 2 * bus_ptr->stops.size() - 1;
            //bus_stats->unique_stops = bus_ptr->isLoop ? (bus_ptr->stops.size() - 1) : bus_ptr->stops.size();
            std::unordered_set<std::string> unique_counter;
            for (auto &stop : bus_ptr->stops)
            {
                unique_counter.insert(stop);
            }
            bus_stats->unique_stops = unique_counter.size();
            auto distance_between_stops_finder = [this](auto &from, auto &to)
            {
                auto from_coord = stop_index_.at(from)->coordinates;
                auto to_coord = stop_index_.at(to)->coordinates;
                return ComputeDistance(from_coord, to_coord);
            };
            double distance = std::transform_reduce(bus_ptr->stops.begin(), bus_ptr->stops.end() - 1,
                                                    bus_ptr->stops.begin() + 1, .0, std::plus<>{}, distance_between_stops_finder);
            bus_stats->route_length = bus_ptr->isLoop ? distance : 2 * distance;
        }
        return result;
    }

private:
    std::deque<Bus>
        buses_;
    std::deque<Stop> stops_;
    std::unordered_map<int, Bus *> bus_index_;
    std::unordered_map<std::string_view, Stop *> stop_index_;
};

std::ostream &operator<<(std::ostream &out, const BusInfo &busInfo)
{
    out << "Bus " << busInfo.bus_number << ": ";
    if (!busInfo.bus_stats)
    {
        out << "not found";
    }
    else
    {
        auto &busStats = busInfo.bus_stats;
        out << busStats->total_stops << " stops on route, ";
        out << busStats->unique_stops << " unique stops, ";
        out << std::setprecision(6) << busStats->route_length << " route length";
    }
    return out;
}