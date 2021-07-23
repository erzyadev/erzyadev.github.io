#include "transport_catalogue.h"
using namespace std;
namespace transport_catalogue
{
    TransportCatalogue::TransportCatalogue(vector<NewStop> &&new_stops, vector<Bus> &&new_buses)
    {
        for (auto &stop : new_stops)
        {
            auto new_stop_ptr = &stops_.emplace_back(stop);

            stop_index_[new_stop_ptr->stop_name] = new_stop_ptr;
        }

        for (auto &bus : new_buses)
        {
            auto new_bus_ptr = &buses_.emplace_back(move(bus));
            bus_index_[new_bus_ptr->bus_number] = new_bus_ptr;
            for (std::string_view stop : new_bus_ptr->stops)
            {
                stop_index_.at(stop)->buses.insert(new_bus_ptr->bus_number);
            }
        }
        for (auto &stop : new_stops)
        {
            auto stop_ptr = stop_index_[stop.stop_name];
            for (auto &distance : stop.distances)
            {
                auto to_stop_ptr = stop_index_[distance.to_stop];
                stop_ptr->distances[to_stop_ptr->stop_name] = distance.distance;
            }
        }

        for (auto &bus : buses_)
            bus_statistics_[bus.bus_number] = CalculateBusStats(bus);
        for (auto &stop : stops_)
        {
            auto bus_sorting_storage = vector(stop.buses.begin(), stop.buses.end());
            sort(bus_sorting_storage.begin(), bus_sorting_storage.end());
            stop_statistics_.insert({string_view{stop.stop_name}, StopData{move(bus_sorting_storage)}});
        }
    }
    double TransportCatalogue::GetGeographicalDistance(std::string_view from, std::string_view to) const
    {
        auto from_coord = stop_index_.at(from)->coordinates;
        auto to_coord = stop_index_.at(to)->coordinates;
        return geo::ComputeDistance(from_coord, to_coord);
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

    BusData TransportCatalogue::CalculateBusStats(const Bus &bus) const
    {

        auto bus_data = BusData{};
        bus_data.total_stops = bus.isLoop ? bus.stops.size() : 2 * bus.stops.size() - 1;
        std::unordered_set<std::string> unique_counter;
        for (auto &stop : bus.stops)
        {
            unique_counter.insert(stop);
        }
        bus_data.unique_stops = unique_counter.size();
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
        double distance = std::transform_reduce(bus.stops.begin(), bus.stops.end() - 1,
                                                bus.stops.begin() + 1, .0, std::plus<>{}, geographical_distance);

        if (bus.isLoop)
        {
            bus_data.route_length = std::transform_reduce(bus.stops.begin(), bus.stops.end() - 1,
                                                          bus.stops.begin() + 1, .0, std::plus<>{}, distance_between_stops_loop);
            bus_data.curvature = bus_data.route_length / distance;
        }
        else
        {
            bus_data.route_length = std::transform_reduce(bus.stops.begin(), bus.stops.end() - 1,
                                                          bus.stops.begin() + 1, .0, std::plus<>{}, distance_between_stops_return);
            bus_data.curvature = bus_data.route_length / distance / 2;
        }

        return bus_data;
    }

    BusStats TransportCatalogue::GetBusStats(std::string_view bus_number) const
    {
        if (auto it = bus_statistics_.find(bus_number); it != bus_statistics_.end())
        {
            return {it->first, it->second};
        }
        else
            return {bus_number, {}};
    }

    StopStats TransportCatalogue::GetStopStats(std::string_view stop_name) const
    {
        if (auto it = stop_statistics_.find(stop_name); it != stop_statistics_.end())
        {
            return {it->first, it->second};
        }
        else
            return {stop_name, {}};
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
        if (!stopInfo.stop_data)
        {
            out << "not found";
        }
        else if (stopInfo.stop_data->buses.empty())
        {
            out << "no buses";
        }
        else
        {
            out << "buses";
            for (const auto &bus : stopInfo.stop_data->buses)
            {
                out << ' ' << bus;
            }
        }
        return out;
    }
}