#include "search_server.h"
#include "log_duration.h"
using namespace std;

const std::map<std::string_view, double> SearchServer::EMPTY_WORD_FREQ_MAP = {};

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int> &ratings)
{
    auto [it, success] = documents_.try_emplace(document_id, DocumentData{0, status});
    if (document_id < 0)
        throw invalid_argument("Negative Document ID");
    if (!success)
        throw invalid_argument("Duplicated Document Id");
    const vector<string_view> words = SplitIntoWordsNoStop(document);
    if (words.size() != 0)
    {
        const double inv_word_count = 1.0 / words.size();
        document_ids_.insert(document_id);
        for (auto word : words)
        {
            //word_to_document_freqs_[word][document_id] += inv_word_count;
            auto word_it = word_to_document_freqs_.lower_bound(word);
            if (word_it == word_to_document_freqs_.end() || word_it->first != word)
            {
                word_it = word_to_document_freqs_.emplace_hint(word_it, word, std::map<int, double>{{document_id, .0}});
                word = word_it->first;
            }
            word_it->second[document_id] += inv_word_count;
            document_to_word_freqs[document_id][word_it->first] += inv_word_count;
        }
        it->second.rating = ComputeAverageRating(ratings);
    }
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const
{
    return MatchDocument(execution::seq, raw_query, document_id);
}

bool SearchServer::IsValidWord(std::string_view word)
{
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c)
                   { return c >= '\0' && c < ' '; });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const
{
    std::vector<std::string_view> words;
    for (std::string_view word : SplitIntoWords(text))
    {
        if (!IsValidWord(word))
        {
            throw std::invalid_argument("Word "s.append(word) + " is invalid"s);
        }
        if (!IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int> &ratings)
{
    if (ratings.empty())
    {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings)
    {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

auto SearchServer::ParseQueryWord(std::string_view text) const -> QueryWord
{
    if (text.empty())
    {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string_view word{text};
    bool is_minus = false;
    if (word[0] == '-')
    {
        is_minus = true;
        word.remove_prefix(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word))
    {
        throw std::invalid_argument("Query word "s.append(text) + " is invalid"s);
    }

    return {word, is_minus, IsStopWord(word)};
}

auto SearchServer::ParseQuery(std::string_view text) const -> Query
{
    Query result;
    for (string_view word : SplitIntoWords(text))
    {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                result.minus_words.insert(query_word.data);
            }
            else
            {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.find(word)->second.size());
}

const map<string_view, double> &SearchServer::GetWordFrequencies(int document_id) const
{
    auto it = document_to_word_freqs.find(document_id);
    if (it == document_to_word_freqs.end())
    {
        return EMPTY_WORD_FREQ_MAP;
    }
    return it->second;
}

void SearchServer::RemoveDocument(int document_id)
{
    document_ids_.erase(document_id);
    documents_.erase(document_id);
    for (auto &[word, freq] : document_to_word_freqs[document_id])
    {
        auto it = word_to_document_freqs_.find(word);
        it->second.erase(document_id);
    }
    document_to_word_freqs.erase(document_id);
}