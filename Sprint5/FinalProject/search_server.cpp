#include "search_server.h"
#include "log_duration.h"
using namespace std;

const std::map<std::string, double> SearchServer::EMPTY_WORD_FREQ_MAP = {};

void SearchServer::AddDocument(int document_id, const string &document, DocumentStatus status, const vector<int> &ratings)
{
    auto [it, success] = documents_.try_emplace(document_id, DocumentData{0, status});
    if (document_id < 0)
        throw invalid_argument("Negative Document ID");
    if (!success)
        throw invalid_argument("Duplicated Document Id");
    const vector<string> words = SplitIntoWordsNoStop(document);
    if (words.size() != 0)
    {
        const double inv_word_count = 1.0 / words.size();
        document_ids_.insert(document_id);
        for (const string &word : words)
        {
            document_to_word_freqs[document_id][word] += inv_word_count;
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        it->second.rating = ComputeAverageRating(ratings);
    }
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string &raw_query, int document_id) const
{
    const auto query = ParseQuery(raw_query);
    LOG_DURATION("Operation time: ");
    std::vector<std::string> matched_words;
    for (const std::string &word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id))
        {
            matched_words.push_back(word);
        }
    }
    for (const std::string &word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id))
        {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsValidWord(const std::string &word)
{
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c)
                   { return c >= '\0' && c < ' '; });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string &text) const
{
    std::vector<std::string> words;
    for (const std::string &word : SplitIntoWords(text))
    {
        if (!IsValidWord(word))
        {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
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

auto SearchServer::ParseQueryWord(const std::string &text) const -> QueryWord
{
    if (text.empty())
    {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word))
    {
        throw std::invalid_argument("Query word "s + text + " is invalid"s);
    }

    return {word, is_minus, IsStopWord(word)};
}

auto SearchServer::ParseQuery(const std::string &text) const -> Query
{
    Query result;
    for (const std::string &word : SplitIntoWords(text))
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

double SearchServer::ComputeWordInverseDocumentFreq(const std::string &word) const
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

const map<string, double> &SearchServer::GetWordFrequencies(int document_id) const
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
        word_to_document_freqs_[word].erase(document_id);
    }
    document_to_word_freqs.erase(document_id);
}