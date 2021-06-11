#include <cassert>
#include <cstddef>
#include <string>
#include <utility>
#include <algorithm>
#include "my_test.h"
template <typename Type>
class SingleLinkedList
{
    // Узел списка
    struct Node
    {
        Node() = default;
        Node(const Type &val, Node *next)
            : value(val), next_node(next)
        {
        }
        Type value;
        Node *next_node = nullptr;
    };
    template <typename ValueType>
    class BasicIterator
    {
        // Класс списка объявляется дружественным, чтобы из методов списка
        // был доступ к приватной области итератора
        friend class SingleLinkedList;

        // Конвертирующий конструктор итератора из указателя на узел списка
        explicit BasicIterator(Node *node) : node_(node)
        {
        }

    public:
        // Объявленные ниже типы сообщают стандартной библиотеке о свойствах этого итератора

        // Категория итератора - forward iterator
        // (итератор, который поддерживает операции инкремента и многократное разыменование)
        using iterator_category = std::forward_iterator_tag;
        // Тип элементов, по которым перемещается итератор
        using value_type = Type;
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        // Тип указателя на итерируемое значение
        using pointer = ValueType *;
        // Тип ссылки на итерируемое значение
        using reference = ValueType &;

        BasicIterator() = default;

        // Конвертирующий конструктор/конструктор копирования
        // При ValueType, совпадающем с Type, играет роль копирующего конструктора
        // При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
        BasicIterator(const BasicIterator<Type> &other) noexcept : node_(other.node_)
        {
        }

        // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
        // пользовательского конструктора копирования, явно объявим оператор = и
        // попросим компилятор сгенерировать его за нас.
        BasicIterator &operator=(const BasicIterator &rhs) = default;

        // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<const Type> &rhs) const noexcept
        {
            return node_ == rhs.node_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator<const Type> &rhs) const noexcept
        {
            return node_ != rhs.node_;
        }

        // Оператор сравнения итераторов (в роли второго аргумента итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<Type> &rhs) const noexcept
        {
            return node_ == rhs.node_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator<Type> &rhs) const noexcept
        {
            return node_ != rhs.node_;
            // Заглушка. Реализуйте оператор самостоятельно
        }

        // Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает ссылку на самого себя
        // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        BasicIterator &operator++() noexcept
        {
            ASSERT_HINT(node_, "Attempt to increment an invalid iterator");

            node_ = node_->next_node;
            return *this;
        }

        // Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка.
        // Возвращает прежнее значение итератора
        // Инкремент итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        BasicIterator operator++(int) noexcept
        {
            ASSERT_HINT(node_, "Attempt to increment an invalid iterator");
            auto old_value{*this};
            node_ = node_->next_node;
            return old_value;
        }

        // Операция разыменования. Возвращает ссылку на текущий элемент
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] reference operator*() const noexcept
        {
            ASSERT_HINT(node_, "Attempt to dereference an invalid iterator");
            return node_->value;
        }

        // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка.
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] pointer operator->() const noexcept
        {
            ASSERT_HINT(node_, "Attempt to dereference an invalid iterator");
            return &node_->value;
        }

    private:
        Node *node_ = nullptr;
    };

public:
    using value_type = Type;
    using reference = value_type &;
    using const_reference = const value_type &;

    // Итератор, допускающий изменение элементов списка
    using Iterator = BasicIterator<Type>;
    // Константный итератор, предоставляющий доступ для чтения к элементам списка
    using ConstIterator = BasicIterator<const Type>;

    // Возвращает итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен end()
    [[nodiscard]] Iterator begin() noexcept
    {
        return Iterator{head_.next_node};
    }

    // Возвращает итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] Iterator end() noexcept
    {
        return Iterator{nullptr};
    }

    // Возвращает константный итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен end()
    // Результат вызова эквивалентен вызову метода cbegin()
    [[nodiscard]] ConstIterator begin() const noexcept
    {
        return Iterator{head_.next_node};
    }

    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    // Результат вызова эквивалентен вызову метода cend()
    [[nodiscard]] ConstIterator end() const noexcept
    {
        return Iterator{nullptr};
    }

    // Возвращает константный итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен cend()
    [[nodiscard]] ConstIterator cbegin() const noexcept
    {
        return Iterator{head_.next_node};
    }

    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cend() const noexcept
    {
        return Iterator{nullptr};
    }

    SingleLinkedList() {}
    // Возвращает количество элементов в списке за время O(1)
    [[nodiscard]] size_t GetSize() const noexcept
    {
        return size_;
    }

    // Сообщает, пустой ли список за время O(1)
    [[nodiscard]] bool IsEmpty() const noexcept
    {
        return head_.next_node == nullptr;
    }

    void PushFront(const Type &value)
    {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    void Clear()
    {
        while (head_.next_node)
            EraseAfter(head_);
    }
    ~SingleLinkedList()
    {
        Clear();
    }

    [[nodiscard]] bool operator==(const SingleLinkedList &rhs) const
    {
        if (size_ != rhs.size_)
            return false;

        return std::equal(begin(), end(), rhs.begin());
    }
    [[nodiscard]] bool operator!=(const SingleLinkedList &rhs) const
    {
        return !operator==(rhs);
    }
    [[nodiscard]] bool operator<(const SingleLinkedList &rhs) const
    {
        return std::lexicographical_compare(begin(), end(), rhs.begin(), rhs.end());
    }
    [[nodiscard]] bool operator>(const SingleLinkedList &rhs) const
    {
        return std::lexicographical_compare(begin(), end(), rhs.begin(), rhs.end(), std::greater<value_type>{});
    }
    [[nodiscard]] bool operator<=(const SingleLinkedList &rhs) const
    {
        return !operator>(rhs);
    }
    [[nodiscard]] bool operator>=(const SingleLinkedList &rhs) const
    {
        return !operator<(rhs);
    }

    void swap(SingleLinkedList &rhs) noexcept
    {
        std::swap(size_, rhs.size_);
        std::swap(head_.next_node, rhs.head_.next_node);
    }

    SingleLinkedList(std::initializer_list<value_type> values)
    {
        try
        {
            Node *node = &head_;
            for (auto &val : values)
            {
                InsertAfter(node, val);
                node = node->next_node;
            }
        }
        catch (std::bad_alloc &e)
        {
            Clear();
            throw;
        }
    }

    SingleLinkedList(const SingleLinkedList &other)
    {

        Node *node = &head_;
        try
        {
            for (auto &val : other)
            {
                InsertAfter(node, val);
                node = node->next_node;
            }
        }
        catch (std::bad_alloc &e)
        {
            Clear();
            throw;
        }
    }
    SingleLinkedList &operator=(const SingleLinkedList &rhs)
    {
        if (this != &rhs)
        {
            SingleLinkedList temp_list(rhs);
            swap(temp_list);
        }

        return *this;
    }
    [[nodiscard]] Iterator before_begin() noexcept
    {

        return Iterator{&head_};
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept
    {
        // Реализуйте самостоятельно
        return ConstIterator{&head_};
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator before_begin() const noexcept
    {
        // Реализуйте самостоятельно
        return cbefore_begin();
    }

    /*
     * Вставляет элемент value после элемента, на который указывает pos.
     * Возвращает итератор на вставленный элемент
     * Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии
     */
    Iterator InsertAfter(ConstIterator pos, const Type &value)
    {
        ASSERT_HINT(pos.node_, "Attempted an insertion after an invalid iterator");
        InsertAfter(pos.node_, value);
        return Iterator(pos.node_->next_node);
    }

    void PopFront() noexcept
    {
        ASSERT_HINT(size_, "Attemtped to pop from an empty list");
        EraseAfter(head_);
    }

    /*
     * Удаляет элемент, следующий за pos.
     * Возвращает итератор на элемент, следующий за удалённым
     */
    Iterator EraseAfter(ConstIterator pos) noexcept
    {
        ASSERT_HINT(pos.node_->next_node, "Attempted to delete from an invalid pointer");
        ASSERT_HINT(size_, "Attempted to delete from an empty list");
        EraseAfter(*pos.node_);
        return Iterator{pos.node_->next_node};
    }

private:
    // Фиктивный узел, используется для вставки "перед первым элементом"
    mutable Node head_;
    size_t size_ = 0;
    void EraseAfter(Node &node) noexcept
    {
        Node *new_next_node = node.next_node->next_node;
        delete node.next_node;
        node.next_node = new_next_node;
        --size_;
    }
    void InsertAfter(Node *node, const value_type &value)
    {
        Node *new_node = new Node{value, node->next_node};
        node->next_node = new_node;
        ++size_;
    }
};

template <typename Type>
void swap(SingleLinkedList<Type> &lhs, SingleLinkedList<Type> &rhs) noexcept
{
    lhs.swap(rhs);
}