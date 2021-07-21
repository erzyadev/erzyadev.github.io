// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"
#include <iostream>

using namespace std;
int main()
{

    cout.precision(6);
    int inputQueriesNum = ReadNumber(cin);
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::vector<string> inputQueriesRaw(inputQueriesNum);
    for (int i = 0; i < inputQueriesNum; ++i)
    {
        getline(cin, inputQueriesRaw[i]);
    }
    auto inputQueries = ParseCreation(inputQueriesRaw);

    TransportCatalogue catalogue;
    for (auto &stopQuery : inputQueries.stops)
    {
        catalogue.AddStop(stopQuery);
    }
    for (auto &busQuery : inputQueries.buses)
    {
        catalogue.AddBus(busQuery);
    }

    int statsQueryNum = ReadNumber(cin);
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < statsQueryNum; ++i)
    {
        int bus_number;
        cin.ignore(4) >> bus_number;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << catalogue.GetStats(bus_number) << endl;
    }
}