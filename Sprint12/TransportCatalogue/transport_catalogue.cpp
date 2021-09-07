#include "transport_catalogue.h"
using namespace std;
namespace transport_catalogue
{
    TransportCatalogue::TransportCatalogue(vector<Stop> &&new_stops, vector<Bus> &&new_buses)
        : buses_(move_iterator(new_buses.begin()), move_iterator(new_buses.end())),
          stops_(move_iterator(new_stops.begin()), move_iterator(new_stops.end()))
    {

        for (auto &stop : stops_)
        {
            stop_index_[stop.stop_name] = &stop;
        }
        for (auto &bus : buses_)
        {
            bus_index_[bus.bus_number] = &bus;
            for (auto &stop : bus.stops)
            {
                stop_index_.at(stop)->buses.insert(bus.bus_number);
            }
        }
        FillDistances();
        for (auto &bus : buses_)
        {
            bus_statistics_[bus.bus_number] = CalculateBusData(bus);
        }
        for (auto &stop : stops_)
        {
            auto bus_sorting_storage = vector(stop.buses.begin(), stop.buses.end());
            sort(bus_sorting_storage.begin(), bus_sorting_storage.end());
            stop_statistics_.insert({string{stop.stop_name}, StopData{move(bus_sorting_storage)}});
        }
    }
    double TransportCatalogue::GetGeographicalDistance(std::string from, std::string to) const
    {
        auto from_coord = stop_index_.at(from)->coordinates;
        auto to_coord = stop_index_.at(to)->coordinates;
        return geo::ComputeDistance(from_coord, to_coord);
    }
    double TransportCatalogue::GetDistanceStopsLoop(std::string from, std::string to) const
    {
        return stop_index_.at(from)->distances.at(to);
    }

    BusData TransportCatalogue::CalculateBusData(const Bus &bus) const
    {

        auto bus_data = BusData{};

        bus_data.total_stops = bus.stops.size();
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

        double distance = std::transform_reduce(bus.stops.begin(), bus.stops.end() - 1,
                                                bus.stops.begin() + 1, .0, std::plus<>{}, geographical_distance);

        bus_data.route_length = std::transform_reduce(bus.stops.begin(), bus.stops.end() - 1,
                                                      bus.stops.begin() + 1, .0, std::plus<>{}, distance_between_stops_loop);
        bus_data.curvature = bus_data.route_length / distance;

        return bus_data;
    }

    BusStats TransportCatalogue::GetBusStats(std::string bus_number) const
    {
        if (auto it = bus_statistics_.find(bus_number); it != bus_statistics_.end())
        {
            return {it->first, it->second};
        }
        else
            return {bus_number, {}};
    }

    StopStats TransportCatalogue::GetStopStats(std::string stop_name) const
    {
        if (auto it = stop_statistics_.find(stop_name); it != stop_statistics_.end())
        {
            return {it->first, it->second};
        }
        else
            return {stop_name, {}};
    }

    std::vector<geo::Coordinates> TransportCatalogue::GetNonemptyStopCoordinates() const
    {
        std::vector<geo::Coordinates> stop_coordinates;
        std::for_each(stops_.begin(), stops_.end(), [&stop_coordinates](const Stop &stop)
                      {
                          if (!stop.buses.empty())
                              stop_coordinates.push_back(stop.coordinates);
                      });
        return stop_coordinates;
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
    void TransportCatalogue::FillDistances()
    {
        for (auto &bus : buses_)
        {
            for (auto it1 = bus.stops.begin(), it2 = next(bus.stops.begin()); it2 != bus.stops.end(); ++it1, ++it2)
            {
                auto &from_name = *it1;
                auto &to_name = *it2;
                Stop *from_ptr = stop_index_.at(from_name);
                Stop *to_ptr = stop_index_.at(to_name);
                auto to_iter = from_ptr->distances.find(to_name);

                if (to_iter == from_ptr->distances.end())
                {
                    from_ptr->distances[to_name] = to_ptr->distances[from_name];
                }
            }
        }
    }
}