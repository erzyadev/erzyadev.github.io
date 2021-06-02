#pragma once
#include "search_server.h"
#include "deque"

class RequestQueue
{
public:
    explicit RequestQueue(const SearchServer &search_server)
        : search_server_(search_server) {}

    template <typename... Args>
    std::vector<Document> AddFindRequest(Args &&...args)
    {
        auto result = search_server_.FindTopDocuments(args...);
        if (requests_.size() == sec_in_day_)
            requests_.pop_front();
        requests_.push_back({result.empty()});
        return result;
    }

    int GetNoResultRequests() const;

private:
    struct QueryResult
    {
        bool is_empty;
    };
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    const SearchServer &search_server_;
};
