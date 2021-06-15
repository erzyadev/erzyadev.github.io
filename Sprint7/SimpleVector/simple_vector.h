#pragma once

#include <initializer_list>
#include "array_ptr.h"
#include <string>
#include <stdexcept>
#include <algorithm>
#include "my_test.h"

using namespace std::string_literals;

struct ReserveProxyObj
{
    size_t capacity;
};

template <typename Type>
class SimpleVector
{

public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : data_(new Type[size]{}), size_(size), capacity_(size)
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value) : SimpleVector(size)
    {
        std::fill(data_.Get(), data_.Get() + size_, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.size())
    {
        std::copy(init.begin(), init.end(), data_.Get());
    }

    SimpleVector(ReserveProxyObj reserve) : data_(new Type[reserve.capacity]{}), size_(0UL), capacity_(reserve.capacity) {}
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept
    {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept
    {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index)
    {
        if (index >= size_)
        {
            std::string message = "Index "s + std::to_string(index) + " exceeds size " + std::to_string(size_);
            throw std::out_of_range(message);
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const
    {
        if (index >= size_)
        {
            std::string message = "Index "s + std::to_string(index) + " exceeds size " + std::to_string(size_);
            throw std::out_of_range(message);
        }
        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        if (new_size <= size_)
            size_ = new_size;
        else if (new_size <= capacity_)
        {
            std::fill(data_.Get() + size_, data_.Get() + new_size, Type{});
        }
        else
        {
            size_t new_capacity = std::max(new_size, 2 * capacity_);
            Reserve(new_capacity);
            size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        return data_.Get();
    }
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
        return data_.Get() + size_;
    }
    SimpleVector(const SimpleVector &other) : SimpleVector(other.size_)
    {
        std::move(other.data_.Get(), other.data_.Get() + size_, data_.Get());
    }
    SimpleVector(SimpleVector &&other) : SimpleVector()
    {
        swap(other);
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        SimpleVector temporary_copy(rhs);

        swap(temporary_copy);
        return *this;
    }
    SimpleVector &operator=(SimpleVector &&rhs)
    {
        swap(rhs);
        return *this;
    }
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора

    template <typename T>
    void PushBack(T &&item)
    {
        if (size_ == capacity_)
        {
            Reserve(std::max(1UL, 2 * size_));
        }
        data_[size_++] = std::forward<T>(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    template <typename T>
    Iterator Insert(ConstIterator pos, T &&value)
    {
        ASSERT_HINT(pos - begin() >= 0) && (end() - pos > 0),"Iterator does not point to vector element");
        Iterator it = const_cast<Iterator>(pos);
        if (size_ == capacity_)
        {
            size_t position = it - begin();
            Reserve(std::max(1UL, 2 * size_));
            it = begin() + position;
        }

        std::move_backward(it, end(), end() + 1);
        ++size_;
        *it = std::forward<T>(value);
        return it;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept
    {
        ASSERT_HINT(size_ != 0, "Attempting to remove an element from empty container");
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos)
    {
        ASSERT_HINT(size_ != 0, "Attempting to remove an element from empty container");
        ASSERT_HINT(pos - begin() >= 0) && (end() - pos > 0),"Iterator does not point to vector element");

        Iterator it = const_cast<Iterator>(pos);

        std::move(it + 1, end(), it);
        --size_;
        return it;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept
    {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity)
    {

        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> new_array{new Type[new_capacity]{}};
            std::move(data_.Get(), data_.Get() + size_, new_array.Get());
            data_.swap(new_array);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> data_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{

    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{

    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{

    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::greater<Type>{});
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{

    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs < rhs);
}
ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return {capacity_to_reserve};
}