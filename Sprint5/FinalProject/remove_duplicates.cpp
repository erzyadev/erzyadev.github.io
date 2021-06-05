#include "remove_duplicates.h"
#include <set>
#include <string>
#include <map>

using namespace std;
namespace
{
    template <typename Key, typename Value>
    auto VectorOfKeys(const map<Key, Value> &m)
    {
        vector<Key> result;
        result.reserve(m.size());
        transform(m.begin(), m.end(), back_inserter(result),
                  [](auto &key_value_pair)
                  { return key_value_pair.first; });
        return result;
    }
}
void RemoveDuplicates(SearchServer &search_server)
{
    set<vector<string>> words_to_docs;
    vector<int> removal_queue;
    for (auto i : search_server)
    {
        auto [_, is_new] = words_to_docs.insert(VectorOfKeys(search_server.GetWordFrequencies(i)));
        if (!is_new)
        {
            cout << "Found duplicate document id " << i << endl;
            removal_queue.push_back(i);
        }
    }

    for (auto i : removal_queue)
        search_server.RemoveDocument(i);
}