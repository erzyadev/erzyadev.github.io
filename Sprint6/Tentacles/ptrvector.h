#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

template <typename T>
class PtrVector
{
public:
    PtrVector() = default;

    // Создаёт вектор указателей на копии объектов из other
    PtrVector(const PtrVector &other) : items_(other.items_.size())
    {
        size_t i = 0;
        try
        {

            for (; i < other.items_.size(); ++i)
            {
                if (other.items_[i])
                    items_[i] = new T{*other.items_[i]};
            }
        }
        catch (std::exception &e)
        {
            for (size_t j = 0; j < i; ++j)
                delete items_[j];
            throw;
        }
    }

    PtrVector &operator=(const PtrVector &rhs)
    {
        if (this != &rhs)
        {
            auto temp_copy = rhs;
            swap(temp_copy.GetItems(), items_);
        }
        return *this;
    }
    // Деструктор удаляет объекты в куче, на которые ссылаются указатели,
    // в векторе items_
    ~PtrVector()
    {
        // Реализуйте тело деструктора самостоятельно
        for (auto ptr : items_)
            delete ptr;
    }

    // Возвращает ссылку на вектор указателей
    vector<T *> &GetItems() noexcept
    {
        // Реализуйте метод самостоятельно
        return items_;
    }

    // Возвращает константную ссылку на вектор указателей
    vector<T *> const &GetItems() const noexcept
    {
        // Реализуйте метод самостоятельно
        return items_;
    }

private:
    vector<T *> items_;
};
