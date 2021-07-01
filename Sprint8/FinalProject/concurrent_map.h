#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>

using std::string_literals::operator""s;

template <typename Key, typename Value>
class ConcurrentMap
{
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access
    {
        Value &ref_to_value;
        std::unique_lock<std::mutex> guard;
    };

    explicit ConcurrentMap(size_t bucket_count) : bucket_count_(bucket_count), maps_(bucket_count), mutexes(bucket_count) {}

    Access operator[](const Key &key)
    {
        auto bucket = GetBucket(key);
        std::unique_lock l{mutexes[bucket]};
        return Access{maps_[bucket][key], move(l)};
    }

    void Erase(Key key)
    {
        auto bucket = GetBucket(key);
        std::lock_guard l(mutexes[bucket]);
        maps_[bucket].erase(key);
    }

    std::map<Key, Value> BuildOrdinaryMap()
    {
        auto result = std::map<Key, Value>{};

        for (int i = 0; i < bucket_count_; ++i)
        {
            auto current_map = std::map<Key, Value>{};
            {
                auto l = std::lock_guard(mutexes[i]);
                current_map = maps_[i];
            }
            {
                result.merge(current_map);
            }
        }
        return result;
    }

    std::vector<std::pair<Key, Value>> MergeTreesToVectorUnsafe() const
    {
        size_t size = transform_reduce(maps_.begin(), maps_.end(), 0, std::plus<>{}, [](auto &&map)
                                       { return map.size(); });
        std::vector<std::pair<Key, Value>> result;
        result.reserve(size);
        for (auto &map : maps_)
        {
            result.insert(result.end(), map.begin(), map.end());
        }
        return result;
    }

private:
    const size_t bucket_count_;
    size_t GetBucket(Key key)
    {
        return static_cast<unsigned long long>(key) % bucket_count_;
    }
    std::vector<std::map<Key, Value>> maps_;
    std::vector<std::mutex> mutexes;
};