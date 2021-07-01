#include "string_processing.h"
using namespace std;

std::vector<std::string_view> SplitIntoWords(std::string_view text)
{
    vector<string_view> words;

    while (!text.empty())
    {
        auto pos = text.find(' ');
        if (pos == 0)
        {
            text.remove_prefix(1);
            continue;
        }
        if (pos != text.npos)
        {
            words.push_back(text.substr(0, pos));
            text.remove_prefix(pos + 1);
        }
        else
        {
            words.push_back(text);
            break;
        }
    }
    return words;
}
