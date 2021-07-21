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
    std::vector<Stop> stops;
    std::vector<Bus> buses;
};

Coordinates ReadCoordinates(std::string_view input)
{
    using namespace std;
    Coordinates result;
    // auto [p, ec] = from_chars(data(input), data(input) + size(input), result.lat);
    // from_chars(p + 2, data(input) + size(input), result.lng);
    istringstream inputStream{string(input)};
    char c;
    inputStream >> result.lat >> c >> result.lng;
    return result;
}

AddQueries ParseCreation(std::vector<std::string> inputs)
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
            new_stop.stop_name = string(query.substr(0, pos));
            query.remove_prefix(pos + 2);
            new_stop.coordinates = ReadCoordinates(query);
            break;
        }
        case 'B':

            query.remove_prefix(4);
            result.buses.emplace_back();
            auto &new_bus = result.buses.back();
            auto [p, ec] = from_chars(data(query), data(query) + query.size(), new_bus.bus_number);
            query.remove_prefix(p - data(query) + 2);
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

std::vector<int> ParseStatsQuery(std::vector<std::string> busQueries)
{
    std::vector<int> result;
    for (auto &query : busQueries)
    {
        result.emplace_back();
        std::from_chars(query.data() + 4, query.data() + query.size(), result.back());
    }
    return result;
}

int ReadNumber(std::istream &input)
{
    int x;
    input >> x;
    return x;
}