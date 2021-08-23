#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

using namespace std;

// реализуйте эту функцию:
void CreateFiles(const string &config_file)
{
    ifstream input(config_file);
    ofstream output_file;
    for (string str; getline(input, str);)
    {
        if (str[0] != '>')
        {
            if (output_file.is_open())
                output_file.close();
            output_file.open(str);
        }
        else
        {
            output_file << str.substr(1) << '\n';
        }
    }
}

string GetLine(istream &in)
{
    string s;
    getline(in, s);
    return s;
}

int main()
{
    ofstream("test_config.txt"s) << "a.txt\n"
                                    ">10\n"
                                    ">abc\n"
                                    "b.txt\n"
                                    ">123"sv;

    CreateFiles("test_config.txt"s);
    ifstream in_a("a.txt"s);
    assert(GetLine(in_a) == "10"s && GetLine(in_a) == "abc"s);

    ifstream in_b("b.txt"s);
    assert(GetLine(in_b) == "123"s);
}
