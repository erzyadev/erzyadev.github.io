// напишите решение с нуля
// код сохраните в свой git-репозиторий
#pragma once
#include "transport_catalogue.h"

void StatsRequest(const TransportCatalogue &catalogue, std::string_view raw_query)
{
    using namespace std;
    switch (raw_query[0])
    {
    case 'S':
        raw_query.remove_prefix(5);
        cout << catalogue.GetStopInfo(raw_query) << endl;
        break;
    case 'B':
        raw_query.remove_prefix(4);
        cout << catalogue.GetBusStats(raw_query) << endl;
    }
}