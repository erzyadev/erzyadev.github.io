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
