#include <cassert>
#include <functional>
#include <string>
#include <memory>

using namespace std;

template <typename T>
class LazyValue
{
public:
    explicit LazyValue(function<T()> init) : init_(init) {}

    bool HasValue() const { return value_ptr != nullptr; }
    const T &Get() const
    {
        if (HasValue())
        {
            return *value_ptr;
        }
        else
        {
            value_ptr.reset(new T{init_()});
            return *value_ptr;
        }
    }

private:
    function<T()> init_;
    mutable unique_ptr<T> value_ptr = nullptr;
};

void UseExample()
{
    const string big_string = "Giant amounts of memory"s;

    LazyValue<string> lazy_string([&big_string]
                                  { return big_string; });

    assert(!lazy_string.HasValue());
    assert(lazy_string.Get() == big_string);
    assert(lazy_string.Get() == big_string);
}

void TestInitializerIsntCalled()
{
    bool called = false;

    {
        LazyValue<int> lazy_int([&called]
                                {
                                    called = true;
                                    return 0;
                                });
    }
    assert(!called);
}

int main()
{
    UseExample();
    TestInitializerIsntCalled();
}