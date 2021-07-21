// напишите решение с нуля
// код сохраните в свой git-репозиторий
#pragma once
#include <map>
#include <string>
#include "transport_catalogue.h"
#include <algorithm>
#include <sstream>
#include <charconv>

using std::string_literals::operator""s;

enum class InputQueryType
{
    Bus,
    Stop,
};

const std::map<std::string, InputQueryType, std::less<>> InputQueryConverter = {
    {"Stop"s, InputQueryType::Stop},
    {"Bus"s, InputQueryType::Bus}};

struct AddQueries
{
    std::vector<NewStop> stops;
    std::vector<Bus> buses;
};

Coordinates ReadCoordinates(std::string_view &input)
{
    using namespace std;
    Coordinates result;
    // auto [p, ec] = from_chars(data(input), data(input) + size(input), result.lat);
    // from_chars(p + 2, data(input) + size(input), result.lng);
    //istringstream inputStream{string(input)};
    //char c;
    //inputStream >> result.lat >> c >> result.lng;

    char *dbl_end;
    result.lat = strtod(data(input), &dbl_end);
    input.remove_prefix(dbl_end - data(input) + 2);
    result.lng = strtod(data(input), &dbl_end);
    input.remove_prefix(dbl_end - data(input));
    return result;
}

DistanceTo GetDistanceTo(std::string_view &query)
{
    using namespace std;
    DistanceTo result;
    auto pos = query.find(',');
    pos = min(pos, query.size());
    auto current_query = query.substr(0, pos);
    query.remove_prefix(pos);
    auto [dist_end, _] = from_chars(data(current_query), data(current_query) + pos, result.distance);
    current_query.remove_prefix((dist_end - data(current_query)) + 5);
    result.to_stop = current_query;
    return result;
}

AddQueries ParseCreation(const std::vector<std::string> &inputs)
{
    using namespace std;
    //sort(inputs.begin(), inputs.end(), greater<>{});
    AddQueries result;
    for (string_view query : inputs)
    {

        switch (query.front())
        {
        case 'S':
        {
            query.remove_prefix(5);
            result.stops.emplace_back();
            auto &new_stop = result.stops.back();
            auto pos = query.find(':');
            new_stop.stop_name = query.substr(0, pos);
            query.remove_prefix(pos + 2);
            new_stop.coordinates = ReadCoordinates(query);
            while (!query.empty())
            {
                query.remove_prefix(2);
                new_stop.distances.push_back(GetDistanceTo(query));
            }
            break;
        }
        case 'B':

            query.remove_prefix(4);
            result.buses.emplace_back();
            auto &new_bus = result.buses.back();
            //auto [p, ec] = from_chars(data(query), data(query) + query.size(), new_bus.bus_number);

            //query.remove_prefix(p - data(query) + 2);
            auto pos = query.find(':');
            new_bus.bus_number = query.substr(0, pos);
            query.remove_prefix(pos + 2);
            auto delim_pos = query.find_first_of(">-");
            char delim = query[delim_pos];
            new_bus.isLoop = delim == '>';
            new_bus.stops.push_back(string(query.substr(0, delim_pos - 1)));
            query.remove_prefix(delim_pos + 2);
            for (auto pos = query.find(delim);; pos = query.find(delim))
            {
                if (pos != query.npos)
                {
                    new_bus.stops.push_back(string(query.substr(0, pos - 1)));
                    query.remove_prefix(pos + 2);
                }
                else
                {
                    new_bus.stops.push_back(string(query));
                    break;
                }
            }
        }
    }
    return result;
}

// std::vector<int> ParseStatsQuery(std::vector<std::string> busQueries)
// {
//     std::vector<int> result;
//     for (auto &query : busQueries)
//     {
//         result.emplace_back();
//         std::from_chars(query.data() + 4, query.data() + query.size(), result.back());
//     }
//     return result;
// }

int ReadNumber(std::istream &input)
{
    int x;
    input >> x;
    return x;
}