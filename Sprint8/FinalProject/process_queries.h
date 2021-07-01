#include <vector>
#include <list>
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer &search_server,
    const std::vector<std::string> &queries);

struct QueryResultWrapper
{
public:
    using ResultType = std::vector<std::vector<Document>>;
    using ItemType = ResultType::value_type;
    using ValueType = ItemType::value_type;
    ResultType results;
    struct ResultIterator
    {
        const ResultType &results;
        ResultType::const_iterator bucket_it;
        ItemType::const_iterator value_it;

        auto &operator++()
        {
            ++value_it;
            if (value_it == bucket_it->end())
            {
                ++bucket_it;
                value_it = bucket_it->begin();
            }
            return *this;
        }

        auto operator++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        const ValueType &operator*()
        {
            return *value_it;
        }
        bool operator==(const ResultIterator &other)
        {
            return (bucket_it == other.bucket_it) &&
                   (bucket_it == results.end() || value_it == other.value_it);
        }
        bool operator!=(ResultIterator &other)
        {
            return !(*this == other);
        }
    };
    ResultIterator begin() const
    {
        auto it = results.begin();
        return it == results.end() ? ResultIterator{results, it, {}} : ResultIterator{results, it, it->begin()};
    }
    ResultIterator end() const
    {
        return {results, results.end(), {}};
    }
};

QueryResultWrapper ProcessQueriesJoined(
    const SearchServer &search_server,
    const std::vector<std::string> &queries);
