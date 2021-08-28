#include "json_builder.h"

using namespace json;
using namespace std;

// Builder &Builder::Value(Node::Value value)
// {
//     if (IsReady())
//         throw logic_error{"Invalid Value call, object was created"};
//     if (!IsValueContext())
//         throw logic_error{"Invalid value context"};
//     if (!root_ && context_stack_.empty())
//     {
//         root_ = value;
//     }
//     else if (IsArrayContext())
//     {
//         auto &current_array = get<Array>(GetContext());
//         current_array.push_back(value);
//     }
//     else if (IsDictContext() && IsKeySet())
//     {
//         auto &current_dict_context = get<DictBuilder>(GetContext());
//         current_dict_context.current_dict.emplace(move(*current_dict_context.current_key),
//                                                   move(value));
//         current_dict_context.current_key = nullopt;
//     }
//     else
//     {
//         throw logic_error{"Invalid value call"};
//     }
//     return *this;
// }

// Builder &Builder::StartArray()
// {
//     if (!IsValueContext())
//         throw logic_error{"Invalid context for array beginning"};
//     context_stack_.push(make_unique<ContainerBuilder>(Array{}));
//     return *this;
// }

// Builder &Builder::EndArray()
// {
//     if (!IsArrayContext())
//     {
//         throw logic_error{"Calling EndArray outside of array context"};
//     }

//     auto current_context = move(context_stack_.top());
//     context_stack_.pop();
//     Value(move(get<Array>(*current_context)));
//     return *this;
// }

// Builder &Builder::StartDict()
// {
//     if (!IsValueContext())
//         throw logic_error{"Invalid context for dict beginning"};
//     context_stack_.push(make_unique<ContainerBuilder>(DictBuilder{}));
//     return *this;
// }
// Builder &Builder::EndDict()
// {
//     if (!IsDictContext() || IsKeySet())
//     {
//         throw logic_error{"Calling EndDict outside of Dict context"};
//     }

//     auto current_context = move(context_stack_.top());
//     context_stack_.pop();
//     Value(move(get<DictBuilder>(*current_context)).current_dict);
//     return *this;
// }

// Builder &Builder::Key(string key)
// {
//     if (!IsDictContext())
//         throw logic_error{"Calling key outside of dict context"};

//     if (IsKeySet())
//     {
//         throw logic_error{"Key has already been set"};
//     }

//     get<DictBuilder>(GetContext()).current_key = move(key);
//     return *this;
// }