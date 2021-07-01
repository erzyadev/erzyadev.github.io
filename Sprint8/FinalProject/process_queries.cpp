#include "process_queries.h"
#include <execution>
#include <numeric>

using namespace std;
std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer &search_server,
    const std::vector<std::string> &queries)
{
    vector<vector<Document>> result(queries.size());
    transform(execution::par, queries.begin(), queries.end(), result.begin(), [&search_server](const auto &query)
              { return search_server.FindTopDocuments(query); });
    return result;
}

QueryResultWrapper ProcessQueriesJoined(
    const SearchServer &search_server,
    const std::vector<std::string> &queries)
{
    return {ProcessQueries(search_server, queries)};
}