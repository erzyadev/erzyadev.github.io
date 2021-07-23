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

namespace transport_catalogue::reader
{

    enum class InputQueryType
    {
        Bus,
        Stop,
    };
    struct AddQueries
    {
        std::vector<NewStop> stops;
        std::vector<Bus> buses;
    };

    geo::Coordinates ReadCoordinates(std::string_view &input);

    DistanceTo ReadDistanceToStop(std::string_view &query);

    AddQueries ParseCreation(const std::vector<std::string> &inputs);

    int ReadNumber(std::istream &input);
}