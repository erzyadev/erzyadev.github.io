#include "json.h"

using namespace std;

namespace json
{

    namespace
    {

        Node LoadNode(istream &input);

        Node LoadArray(istream &input)
        {
            Array result;

            for (char c; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input)
                throw ParsingError{"Array reading error"};
            return Node(move(result));
        }

        Node LoadNumber(std::istream &input)
        {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char]
            {
                if (!std::isdigit(input.peek()))
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek()))
                {
                    read_char();
                }
            };
            if (input.peek() == 'n')
            {
                char str[5];
                input.get(str, 5);
                if ("null"sv == str)
                {
                    return Node(nullptr);
                }
                else
                {
                    throw ParsingError{"Failed to read a null element"};
                }
            }

            if (input.peek() == 't')
            {
                char str[5];
                input.get(str, 5);
                if ("true"sv == str)
                {
                    return Node{true};
                }
                else
                {
                    throw ParsingError{"Failed to read a null element"};
                }
            }

            if (input.peek() == 'f')
            {
                char str[6];
                input.get(str, 6);
                if ("false"sv == str)
                {
                    return Node{false};
                }
                else
                {
                    throw ParsingError{"Failed to read a null element"};
                }
            }

            if (input.peek() == '-')
            {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0')
            {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try
            {
                if (is_int)
                {
                    // Сначала пробуем преобразовать строку в int
                    try
                    {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...)
                    {
                        // В случае неудачи, например, при переполнении
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...)
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream &input)
        {
            string line;
            char c;
            for (c = input.get(); !!input && c != '\"'; c = input.get())
            {
                if (c == '\\')
                {
                    switch (input.peek())
                    {
                    case 't':
                        line.push_back('\t');
                        input.ignore(1);
                        break;

                    case 'n':
                        line.push_back('\n');
                        input.ignore(1);
                        break;

                    case 'r':
                        line.push_back('\r');
                        input.ignore(1);
                        break;

                    case '\"':
                        line.push_back('\"');
                        input.ignore(1);
                        break;

                    case '\\':
                        line.push_back('\\');
                        input.ignore(1);
                        break;
                    default:
                        line.push_back(c);
                        char after_slash;
                        input >> after_slash;
                        line.push_back(after_slash);
                    }
                }
                else
                {
                    line.push_back(c);
                }
            }
            if (!input)
                throw ParsingError{"String reading error"};

            //            getline(input, line, '"');
            return Node(move(line));
        }

        Node LoadDict(istream &input)
        {
            Dict result;

            for (char c; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if (!input)
                throw ParsingError{"Dictionary reading error"};
            return Node(move(result));
        }

        Node LoadNode(istream &input)
        {
            char c;
            input >> c;

            if (c == '[')
            {
                return LoadArray(input);
            }
            else if (c == '{')
            {
                return LoadDict(input);
            }
            else if (c == '"')
            {
                return LoadString(input);
            }
            else
            {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    } // namespace

    Node::Node(Array array)
        : data_(move(array))
    {
    }

    Node::Node(Dict map)
        : data_(move(map))
    {
    }

    Node::Node(int value)
        : data_(value)
    {
    }

    Node::Node(double value)
        : data_(value)
    {
    }
    Node::Node(string value)
        : data_(move(value))
    {
    }
    Node::Node(nullptr_t)
        : data_() {}

    Node::Node(bool value)
        : data_{value}
    {
    }

    const Array &Node::AsArray() const
    {
        if (IsArray())
            return std::get<Array>(data_);
        else
            throw std::logic_error{"Invalid access to an array node"};
    }

    const Dict &Node::AsMap() const
    {
        if (IsMap())
            return std::get<Dict>(data_);
        else
            throw std::logic_error{"Invalid access to a dictionary node"};
    }

    int Node::AsInt() const
    {
        if (IsInt())
            return std::get<int>(data_);
        else
            throw std::logic_error{"Invalid access to a number (integer) node"};
    }

    double Node::AsPureDouble() const
    {
        if (IsPureDouble())
            return std::get<double>(data_);
        else
            throw std::logic_error{"Invalid access to a number (pure double) node"};
    }
    double Node::AsDouble() const
    {
        if (IsDouble())
        {
            if (IsInt())
            {
                return static_cast<double>(std::get<int>(data_));
            }
            else
            {
                return std::get<double>(data_);
            }
        }
        else
            throw std::logic_error{"Invalid access to a number (double) node"};
    }

    const string &Node::AsString() const
    {
        if (std::holds_alternative<std::string>(data_))
            return std::get<std::string>(data_);
        else
            throw std::logic_error{"Invalid access to a string node"};
    }

    bool Node::AsBool() const
    {
        if (std::holds_alternative<bool>(data_))
            return std::get<bool>(data_);
        else
            throw std::logic_error{"Invalid access to a bool node"};
    }

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    Document Load(istream &input)
    {
        return Document{LoadNode(input)};
    }

    void Print(const Document &doc, std::ostream &output)
    {
        (void)&doc;
        (void)&output;

        // Реализуйте функцию самостоятельно
        const Node &root = doc.GetRoot();
        output << root;
    }

    std::ostream &Node::NodeDataPrinter::operator()(const Array &array)
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

    std::ostream &Node::NodeDataPrinter::operator()(const Dict &dict)
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

    std::ostream &Node::NodeDataPrinter::operator()(std::nullptr_t)
    {
        return out << "null";
    }

    std::ostream &Node::NodeDataPrinter::operator()(int value)
    {
        return out << value;
    }

    std::ostream &Node::NodeDataPrinter::operator()(double value)
    {
        return out << value;
    }

    std::ostream &Node::NodeDataPrinter::operator()(bool value)
    {
        return out << std::boolalpha << value;
    }

    std::ostream &Node::NodeDataPrinter::operator()(const string &str)
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
} // namespace json