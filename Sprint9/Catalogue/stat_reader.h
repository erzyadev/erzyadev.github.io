
#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue::statistics
{
    void StatsRequest(const TransportCatalogue &catalogue, std::string_view raw_query);
}