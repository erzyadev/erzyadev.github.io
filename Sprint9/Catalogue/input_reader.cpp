#include "input_reader.h"
using namespace std;
namespace transport_catalogue::reader
{
    geo::Coordinates ReadCoordinates(std::string_view &input)
    {

        geo::Coordinates result;

        char *dbl_end;
        result.lat = strtod(data(input), &dbl_end);
        input.remove_prefix(dbl_end - data(input) + 2);
        result.lng = strtod(data(input), &dbl_end);
        input.remove_prefix(dbl_end - data(input));

        return result;
    }

    DistanceTo ReadDistanceToStop(std::string_view &query)
    {

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
                    new_stop.distances.push_back(ReadDistanceToStop(query));
                }
                break;
            }
            case 'B':

                query.remove_prefix(4);
                auto pos = query.find(':');
                result.buses.emplace_back(query.substr(0, pos));
                auto &new_bus = result.buses.back();

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

    int ReadNumber(std::istream &input)
    {
        int x;
        input >> x;
        return x;
    }
}