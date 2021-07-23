#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"
#include <fstream>
#include "stat_reader.h"
#include <iostream>

using namespace std;
int main()
{
#ifdef MY_DEBUG
    ifstream in("../input.txt");
#else
    auto &in = cin;
#endif
    using namespace transport_catalogue;
    cout.precision(6);
    int inputQueriesNum = reader::ReadNumber(in);
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::vector<string> inputQueriesRaw(inputQueriesNum);
    for (int i = 0; i < inputQueriesNum; ++i)
    {
        getline(in, inputQueriesRaw[i]);
    }
    auto inputQueries = reader::ParseCreation(inputQueriesRaw);

    auto catalogue = TransportCatalogue{move(inputQueries.stops), move(inputQueries.buses)};
    int statsQueryNum = reader::ReadNumber(in);
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < statsQueryNum; ++i)
    {
        string s;
        getline(in, s);
        statistics::StatsRequest(catalogue, s);
    }
}