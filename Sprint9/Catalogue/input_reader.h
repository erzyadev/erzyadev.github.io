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

    const std::map<std::string, InputQueryType, std::less<>> InputQueryConverter = {
        {"Stop"s, InputQueryType::Stop},
        {"Bus"s, InputQueryType::Bus}};

    struct AddQueries
    {
        std::vector<NewStop> stops;
        std::vector<Bus> buses;
    };

    Coordinates ReadCoordinates(std::string_view &input);

    DistanceTo GetDistanceTo(std::string_view &query);

    AddQueries ParseCreation(const std::vector<std::string> &inputs);

    int ReadNumber(std::istream &input);
}