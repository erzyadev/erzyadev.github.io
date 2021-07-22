#include "transport_catalogue.h"

namespace transport_catalogue
{
    void TransportCatalogue::AddStop(NewStop stop)
    {
        auto new_stop_it = stop_index_.find(stop.stop_name);
        Stop *new_stop_ptr;
        if (new_stop_it == stop_index_.end())
        {
            new_stop_ptr = CreateStopByName(stop.stop_name);
        }
        else
        {
            new_stop_ptr = new_stop_it->second;
        }
        auto &new_stop = *new_stop_ptr;
        new_stop.coordinates = stop.coordinates;
        for (auto &distance : stop.distances)
        {
            auto it = stop_index_.find(distance.to_stop);
            Stop *stop_ptr;
            if (it == stop_index_.end())
            {
                stop_ptr = CreateStopByName(distance.to_stop);
            }
            else
            {
                stop_ptr = it->second;
            }
            new_stop.distances[stop_ptr->stop_name] = distance.distance;
        }
    }
    void TransportCatalogue::AddBus(Bus bus)
    {
        buses_.push_back(bus);
        bus_index_[buses_.back().bus_number] = &buses_.back();
        for (std::string_view stop : buses_.back().stops)
        {
            stop_index_.at(stop)->buses.insert(buses_.back().bus_number);
        }
    }
    double TransportCatalogue::GetGeographicalDistance(std::string_view from, std::string_view to) const
    {
        auto from_coord = stop_index_.at(from)->coordinates;
        auto to_coord = stop_index_.at(to)->coordinates;
        return ComputeDistance(from_coord, to_coord);
    }
    double TransportCatalogue::GetDistanceStopsLoop(std::string_view from, std::string_view to) const
    {
        Stop *from_ptr = stop_index_.at(from);
        auto to_iter = from_ptr->distances.find(to);
        if (to_iter != from_ptr->distances.end())
        {
            return to_iter->second;
        }
        else
        {
            return stop_index_.at(to)->distances.at(from);
        }
    }
    double TransportCatalogue::GetDistanceStopsReturn(std::string_view from, std::string_view to) const
    {
        Stop *from_ptr = stop_index_.at(from);
        Stop *to_ptr = stop_index_.at(to);
        auto to_iter = from_ptr->distances.find(to);
        auto from_iter = to_ptr->distances.find(from);
        if (to_iter != from_ptr->distances.end())
        {
            if (from_iter != to_ptr->distances.end())
            {
                return to_iter->second + from_iter->second;
            }
            else
                return 2 * to_iter->second;
        }
        else
        {
            return 2 * from_iter->second;
        }
    }

    BusStats TransportCatalogue::GetBusStats(std::string_view bus_number) const
    {

        BusStats result{bus_number, std::nullopt};
        if (auto it = bus_index_.find(bus_number); it != bus_index_.end())
        {
            auto &bus_stats = result.bus_stats;
            bus_stats.emplace();
            auto bus_ptr = it->second;
            bus_stats->total_stops = bus_ptr->isLoop ? bus_ptr->stops.size() : 2 * bus_ptr->stops.size() - 1;
            std::unordered_set<std::string> unique_counter;
            for (auto &stop : bus_ptr->stops)
            {
                unique_counter.insert(stop);
            }
            bus_stats->unique_stops = unique_counter.size();
            auto geographical_distance = [this](auto &from, auto &to)
            {
                return GetGeographicalDistance(from, to);
            };
            auto distance_between_stops_loop = [this](auto &from, auto &to)
            {
                return GetDistanceStopsLoop(from, to);
            };
            auto distance_between_stops_return = [this](auto &from, auto &to)
            {
                return GetDistanceStopsReturn(from, to);
            };
            double distance = std::transform_reduce(bus_ptr->stops.begin(), bus_ptr->stops.end() - 1,
                                                    bus_ptr->stops.begin() + 1, .0, std::plus<>{}, geographical_distance);

            double route_length;
            if (bus_ptr->isLoop)
            {
                bus_stats->route_length = std::transform_reduce(bus_ptr->stops.begin(), bus_ptr->stops.end() - 1,
                                                                bus_ptr->stops.begin() + 1, .0, std::plus<>{}, distance_between_stops_loop);
                bus_stats->curvature = bus_stats->route_length / distance;
            }
            else
            {
                bus_stats->route_length = std::transform_reduce(bus_ptr->stops.begin(), bus_ptr->stops.end() - 1,
                                                                bus_ptr->stops.begin() + 1, .0, std::plus<>{}, distance_between_stops_return);
                bus_stats->curvature = bus_stats->route_length / distance / 2;
            }
        }
        return result;
    }

    StopStats TransportCatalogue::GetStopStats(std::string_view stop_name) const
    {
        using namespace std;
        StopStats result{stop_name, nullopt};
        if (auto it = stop_index_.find(stop_name); it != stop_index_.end())
        {
            result.stop_stats = StopData{};
            for (string_view bus : it->second->buses)
            {
                result.stop_stats->buses.push_back(bus);
            }
            std::sort(result.stop_stats->buses.begin(), result.stop_stats->buses.end());
        }
        return result;
    }

    Stop *TransportCatalogue::CreateStopByName(std::string_view stop_name)
    {
        Stop *new_stop_ptr = &stops_.emplace_back();
        new_stop_ptr->stop_name = std::string(stop_name);
        stop_index_[new_stop_ptr->stop_name] = new_stop_ptr;
        return new_stop_ptr;
    }

    std::ostream &operator<<(std::ostream &out, const BusStats &busInfo)
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
            out << std::setprecision(6) << busStats->route_length << " route length, ";
            out << std::setprecision(6) << busStats->curvature << " curvature";
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, const StopStats &stopInfo)
    {
        out << "Stop " << stopInfo.stop_name << ": ";
        if (!stopInfo.stop_stats)
        {
            out << "not found";
        }
        else if (stopInfo.stop_stats->buses.empty())
        {
            out << "no buses";
        }
        else
        {
            out << "buses";
            for (const auto &bus : stopInfo.stop_stats->buses)
            {
                out << ' ' << bus;
            }
        }
        return out;
    }
}