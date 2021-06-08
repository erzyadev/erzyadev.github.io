#include "ptrvector.h"
#include "scopedptr.h"
using namespace std;

// Щупальце
class Tentacle
{
public:
    explicit Tentacle(int id) noexcept
        : id_(id)
    {
    }

    int GetId() const noexcept
    {
        return id_;
    }

    Tentacle *GetLinkedTentacle() const noexcept
    {
        return linked_tentacle_;
    }
    void LinkTo(Tentacle &tentacle) noexcept
    {
        linked_tentacle_ = &tentacle;
    }
    void Unlink() noexcept
    {
        linked_tentacle_ = nullptr;
    }

private:
    int id_ = 0;
    Tentacle *linked_tentacle_ = nullptr;
};

// Осьминог
class Octopus
{
public:
    Octopus()
        : Octopus(8)
    {
    }

    explicit Octopus(int num_tentacles)
    {

        for (int i = 1; i <= num_tentacles; ++i)
        {
            ScopedPtr t{new Tentacle(i)};                   // Может выбросить исключение bad_alloc
            tentacles_.GetItems().push_back(t.GetRawPtr()); // Может выбросить исключение bad_alloc

            // Обнуляем указатель на щупальце, которое уже добавили в tentacles_,
            // чтобы не удалить его в обработчике catch повторно
            t.Release();
        }
    }

    // Octopus(const Octopus &other) : tentacles_(other.tentacles_) {}

    // Octopus& operator=(const Octopus& rhs) {

    // }
    // Добавляет новое щупальце с идентификатором,
    // равным (количество_щупалец + 1):
    // 1, 2, 3, ...
    // Возвращает ссылку на добавленное щупальце
    Tentacle &AddTentacle()
    {
        ScopedPtr t(new Tentacle{(int)tentacles_.GetItems().size() + 1});
        tentacles_.GetItems().push_back(t.GetRawPtr());
        t.Release();
        return *tentacles_.GetItems().back();
    }

    int GetTentacleCount() const noexcept
    {
        return static_cast<int>(tentacles_.GetItems().size());
    }

    const Tentacle &GetTentacle(size_t index) const
    {
        return *tentacles_.GetItems().at(index);
    }
    Tentacle &GetTentacle(size_t index)
    {
        return *tentacles_.GetItems().at(index);
    }

private:
    PtrVector<Tentacle> tentacles_;
};
