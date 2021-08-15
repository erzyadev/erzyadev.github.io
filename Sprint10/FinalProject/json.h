#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json
{

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };
#define explicit
    class Node
    {
    public:
        Node() = default;
        explicit Node(Array array);
        explicit Node(Dict map);
        explicit Node(int value);
        explicit Node(double value);
        explicit Node(std::string value);
        explicit Node(nullptr_t);
        explicit Node(bool value);

        const Array &AsArray() const;
        const Dict &AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        double AsPureDouble() const;
        const std::string &AsString() const;
        bool AsBool() const;

        bool IsNull() const { return std::holds_alternative<nullptr_t>(data_); }
        bool IsDouble() const { return IsInt() || IsPureDouble(); }
        bool IsPureDouble() const { return std::holds_alternative<double>(data_); }
        bool IsString() const { return std::holds_alternative<std::string>(data_); }
        bool IsMap() const { return std::holds_alternative<Dict>(data_); }
        bool IsInt() const { return std::holds_alternative<int>(data_); }
        bool IsArray() const { return std::holds_alternative<Array>(data_); }
        bool IsBool() const { return std::holds_alternative<bool>(data_); }

        bool operator==(const Node &rhs) const { return data_ == rhs.data_; }
        bool operator!=(const Node &rhs) const { return !(*this == rhs); }

    private:
        // Array as_array_;
        // Dict as_map_;
        // int as_int_ = 0;
        // std::string as_string_;
        std::variant<nullptr_t, Array, Dict, int, double, bool, std::string> data_;
        friend std::ostream &operator<<(std::ostream &out, const Node &node);

        struct NodeDataPrinter
        {
            std::ostream &out;
            std::ostream &operator()(const Array &array)
            {
                out << '[';
                if (!array.empty())
                {
                    out << *array.begin();
                    for (auto node_it = array.begin() + 1; node_it != array.end(); ++node_it)
                    {
                        out << ",";
                        out << *node_it;
                    }
                }
                return out << ']';
            }
            std::ostream &operator()(const Dict &dict)
            {
                out << '{';
                if (!dict.empty())
                {

                    operator()(dict.begin()->first) << ":" << dict.begin()->second;
                    for (auto node_it = next(dict.begin()); node_it != dict.end(); ++node_it)
                    {
                        out << ",";
                        operator()(node_it->first) << ":" << node_it->second;
                    }
                }

                return out << '}';
            }
            std::ostream &operator()(nullptr_t)
            {
                return out << "null";
            }
            std::ostream &operator()(int value)
            {
                return out << value;
            }
            std::ostream &operator()(double value)
            {
                return out << value;
            }

            std::ostream &operator()(bool value)
            {
                return out << std::boolalpha << value;
            }
            std::ostream &operator()(const std::string &str)
            {
                out << "\"";

                for (char c : str)
                {

                    switch (c)
                    {
                    case '\t':
                        out << "\\t";
                        break;

                    case '\n':
                        out << "\\n";
                        break;

                    case '\r':
                        out << "\\r";
                        break;

                    case '\"':
                        out << "\\\"";
                        break;

                    case '\\':
                        out << "\\\\";
                        break;
                    default:
                        out << c;
                    }
                }

                return out << "\"";
            }
        };

        friend std::ostream &operator<<(std::ostream &out, const Node &node)
        {
            return std::visit(NodeDataPrinter{out}, node.data_);
        }
    };

    class Document
    {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;
        bool operator==(const Document &rhs) { return root_ == rhs.root_; }
        bool operator!=(const Document &rhs) { return root_ != rhs.root_; }

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

} // namespace json