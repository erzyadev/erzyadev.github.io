// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"
#include <fstream>
#include "stat_reader.h"
#include <iostream>

using namespace std;
int main()
{
#ifdef NDEBUG
    ifstream in("input.txt");
#else
    auto &in = cin;
#endif
    cout.precision(6);
    int inputQueriesNum = ReadNumber(in);
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::vector<string> inputQueriesRaw(inputQueriesNum);
    for (int i = 0; i < inputQueriesNum; ++i)
    {
        getline(in, inputQueriesRaw[i]);
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

    int statsQueryNum = ReadNumber(in);
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < statsQueryNum; ++i)
    {
        string s;
        getline(in, s);
        StatsRequest(catalogue, s);
    }
}