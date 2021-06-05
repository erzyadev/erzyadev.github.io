#include "request_queue.h"

int RequestQueue::GetNoResultRequests() const
{
    return std::count_if(requests_.begin(), requests_.end(), [](auto qr)
                         { return qr.is_empty; });
}