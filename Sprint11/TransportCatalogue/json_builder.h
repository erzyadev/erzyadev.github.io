#include "json.h"

#include <optional>
#include <stdexcept>
#include <stack>
#include <memory>
namespace json
{
    // class Builder;

    template <typename>
    class ArrayContext;

    template <typename>
    class DictContext;

    //CRTP base class implemeting common StartDict() and StartArray() methods
    template <typename T>
    class BaseContext
    {
    public:
        DictContext<T> StartDict()
        {
            return DictContext<T>{static_cast<T &>(*this)};
        }
        ArrayContext<T> StartArray()
        {
            return ArrayContext<T>{static_cast<T &>(*this)};
        }
    };

    template <typename PrevContext>
    class ArrayContext : public BaseContext<ArrayContext<PrevContext>>
    {
    public:
        ArrayContext(PrevContext &caller) : prev_context_{caller} {};
        ArrayContext &Value(Node::Value value)
        {
            values_.emplace_back(move(value));
            return *this;
        }
        //the decltype(auto) here is where the magic happens
        decltype(auto) EndArray()
        {
            return prev_context_.Value(values_);
        }

    private:
        PrevContext &prev_context_;
        Array values_;
    };

    template <typename PrevContext>
    class DictContext;

    template <typename PrevContext>
    class DictContext
    {
    public:
        DictContext(PrevContext &caller) : prev_context_{caller} {};

        //one more magic auto
        decltype(auto) EndDict()
        {
            return prev_context_.Value(dict_);
        }

        class KeyContext : public BaseContext<KeyContext>
        {
        public:
            KeyContext(DictContext &current_dict, std::string key)
                : current_dict_(current_dict), key_{std::move(key)} {}
            DictContext &Value(Node::Value value)
            {
                current_dict_.dict_.emplace(move(key_), move(value));
                return current_dict_;
            }

        private:
            DictContext &current_dict_;
            std::string key_;
        };
        KeyContext Key(std::string key)
        {
            return {*this, move(key)};
        }

    private:
        PrevContext &prev_context_;
        Dict dict_;
    };

    class ReadyNodeContext
    {

    public:
        ReadyNodeContext(Node::Value &&value) : value_(move(value)) {}
        Node Build()
        {
            return move(value_);
        }

    private:
        Node::Value value_;
    };

    class Builder : public BaseContext<Builder>
    {
    public:
        ReadyNodeContext Value(Node::Value value)
        {
            return value;
        }
    };
}