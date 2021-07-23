#include "stat_reader.h"

namespace transport_catalogue::statistics
{
    void StatsRequest(const TransportCatalogue &catalogue, std::string_view raw_query)
    {
        using namespace std;
        switch (raw_query[0])
        {
        case 'S':
            raw_query.remove_prefix(5);
            cout << catalogue.GetStopStats(raw_query) << endl;
            break;
        case 'B':
            raw_query.remove_prefix(4);
            cout << catalogue.GetBusStats(raw_query) << endl;
        }
    }
}