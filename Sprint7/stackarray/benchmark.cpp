#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <numeric>
using namespace std;
#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

class LogDuration
{
public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string &id, std::ostream &dst_stream = std::cerr)
        : id_(id), dst_stream_(dst_stream)
    {
    }

    ~LogDuration()
    {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        dst_stream_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string id_;
    const Clock::time_point start_time_ = Clock::now();
    std::ostream &dst_stream_;
};

template <typename T, size_t N>
class StackVector
{
public:
    explicit StackVector(size_t a_size = 0)
    {
        if (a_size > N)
            throw std::invalid_argument("Size exceeds capacity");
        size_ = a_size;
        std::fill(data_.begin(), data_.begin() + a_size, T{});
    }

    T &
    operator[](size_t index)
    {

        return data_[index];
    }
    const T &operator[](size_t index) const
    {
        return data_[index];
    }

    T *begin() { return data_.begin(); }
    T *end() { return data_.begin() + size_; }
    const T *begin() const { return data_.begin(); }
    const T *end() const { return data_.begin() + size_; }

    size_t Size() const { return size_; }
    size_t Capacity() const { return N; }

    void PushBack(const T &value)
    {
        if (size_ == N)
            throw std::overflow_error("Attempting to push back into a vector at full capaclity");
        data_[size_++] = value;
    }
    T PopBack()
    {
        if (size_ == 0)
            throw std::underflow_error("Attempted to remove elements from empty vector");

        return std::move(data_[--size_]);
    }

private:
    size_t size_;
    std::array<T, N> data_;
};

int main()
{
    cerr << "Running benchmark..."s << endl;

    const size_t max_size = 2500;

    default_random_engine re;
    uniform_int_distribution<int> value_gen(1, max_size);
    const int ITERS = 50000;
    vector<vector<int>> test_data(ITERS);
    for (auto &cur_vec : test_data)
    {
        cur_vec.resize(value_gen(re));
        for (int &x : cur_vec)
        {
            x = value_gen(re);
        }
    }
    vector<size_t> indices(ITERS);
    for (size_t i = 0; i < ITERS; ++i)
    {
        uniform_int_distribution<int> value_geni(1, test_data[i].size());
        indices[i] = value_geni(re);
    }
    uint32_t sum = 0;
    {

        LOG_DURATION("vector w/o reserve");
        for (auto &cur_vec : test_data)
        {
            vector<int> v;
            for (int x : cur_vec)
            {
                v.push_back(x);
            }
            sum += accumulate(v.begin(), v.end(), 0UL);
        }
    }
    cout << sum << endl;
    sum = 0;
    {
        LOG_DURATION("vector with reserve");
        for (auto &cur_vec : test_data)
        {
            vector<int> v;
            v.reserve(cur_vec.size());
            for (int x : cur_vec)
            {
                v.push_back(x);
            }
            sum += accumulate(v.begin(), v.end(), 0UL);
        }
    }
    cout << sum << endl;
    sum = 0;
    {
        LOG_DURATION("StackVector");
        for (auto &cur_vec : test_data)
        {
            StackVector<int, max_size> v;
            for (int x : cur_vec)
            {
                v.PushBack(x);
            }
            cur_vec.clear();
            sum += accumulate(v.begin(), v.end(), 0UL);
        }
    }
    cout << sum << endl;
}