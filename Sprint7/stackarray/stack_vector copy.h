#pragma once

#include <array>
#include <stdexcept>

template <typename T, size_t N>
class StackVector
{
public:
    explicit StackVector(size_t a_size = 0)
    {
        if (a_size > N)
            throw std::invalid_argument("Size exceeds capacity");
        end_ = data_.begin() + a_size;
        std::fill(data_.begin(), end_, T{});
    }

    StackVector(const StackVector &other) : data_(other.data_), end_(data_.begin() + other.Size()) {}
    StackVector &operator=(const StackVector &rhs)
    {
        data_ = rhs.data_;
        end_ = rhs.Size() + data_.begin();
        return *this;
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
    T *end() { return end_; }
    const T *begin() const { return data_.begin(); }
    const T *end() const { return end_; }

    size_t Size() const { return end_ - data_.begin(); }
    size_t Capacity() const { return N; }

    void PushBack(const T &value)
    {
        if (end_ == data_.end())
            throw std::overflow_error("Attempting to push back into a vector at full capaclity");
        *end_ = value;
        ++end_;
    }
    T PopBack()
    {
        if (end_ == data_.begin())
            throw std::underflow_error("Attempted to remove elements from empty vector");

        return std::move(*(--end_));
    }

private:
    T *end_;
    std::array<T, N> data_;
};
