
#pragma once
#include <execution>
#include <numeric>
#include <stdexcept>
#include <set>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include "string_processing.h"
#include "document.h"
#include "concurrent_map.h"
using namespace std::string_literals;
//using std::invalid_argument;
constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer
{
public:
    //constructs a server from a container with
    //stop words
    template <typename StringContainer,
              typename = std::enable_if_t<
                  std::is_same_v<typename StringContainer::value_type, std::string_view> ||
                      std::is_same_v<typename StringContainer::value_type, std::string>,
                  int>>
    explicit SearchServer(const StringContainer &stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) // Extract non-empty stop words
    {
        if (auto it = std::find_if_not(stop_words_.begin(), stop_words_.end(), IsValidWord);
            it != stop_words_.end())
        {
            throw std::invalid_argument("Invalid word encountered: "s + *it);
        }
    }

    //construct a server from a string with
    //space-separated stop words
    explicit SearchServer(const std::string_view stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)) // Invoke delegating constructor
                                                        // from a string container
    {
    }

    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int> &ratings);

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy &policy, std::string_view raw_query, DocumentPredicate document_predicate) const
    {
        using namespace std;

        const auto query = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(policy, query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(), [](const Document &lhs, const Document &rhs)
             {
                 if (abs(lhs.relevance - rhs.relevance) < TOLERANCE)
                 {
                     return lhs.rating > rhs.rating;
                 }
                 else
                 {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    template <typename ExecutionPolicy>
    std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, std::vector<Document>> FindTopDocuments(const ExecutionPolicy &policy, std::string_view raw_query, DocumentStatus status) const
    {
        return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating)
                                { return document_status == status; });
    }

    template <typename ExecutionPolicy>
    std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, std::vector<Document>> FindTopDocuments(const ExecutionPolicy &policy, std::string_view raw_query) const
    {
        return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
    }

    template <typename... Args>
    std::vector<Document> FindTopDocuments(std::string_view query, Args &&...args) const
    {
        return FindTopDocuments(std::execution::seq, query, std::forward<Args>(args)...);
    }

    int GetDocumentCount() const
    {
        return documents_.size();
    }

    auto begin() const
    {
        return document_ids_.begin();
    }
    auto end() const
    {
        return document_ids_.end();
    }

    const std::map<std::string_view, double> &GetWordFrequencies(int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;
    template <typename ExPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExPolicy policy, std::string_view raw_query, int document_id) const
    {
        const auto query = ParseQuery(raw_query);

        auto is_word_in_document = [this, document_id](std::string_view word)
        {
            auto it = this->word_to_document_freqs_.find(word);
            if (it != this->word_to_document_freqs_.end() &&
                it->second.find(document_id) != it->second.end())
                return true;
            return false;
        };
        auto minus_found_it = std::find_if(policy, query.minus_words.begin(), query.minus_words.end(), is_word_in_document);
        if (minus_found_it != query.minus_words.end())
            return {{}, documents_.at(document_id).status};
        std::vector<std::string_view> matched_words;
        std::copy_if(policy, query.plus_words.begin(), query.plus_words.end(), back_inserter(matched_words), is_word_in_document);
        return {matched_words, documents_.at(document_id).status};
    }
    void RemoveDocument(int document_id);

    template <typename ExPolicy>
    void RemoveDocument(ExPolicy policy, int document_id);

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    const std::set<std::string, std::less<>> stop_words_;
    std::map<std::string, std::map<int, double>, std::less<>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    std::map<int, std::map<std::string_view, double>> document_to_word_freqs;
    static const std::map<std::string_view, double> EMPTY_WORD_FREQ_MAP;
    static constexpr double TOLERANCE = 1e-6;

    bool IsStopWord(std::string_view word) const
    {
        return stop_words_.count(word) > 0;
    }

    static bool IsValidWord(std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int> &ratings);

    struct QueryWord
    {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view text) const;

    struct Query
    {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    Query ParseQuery(std::string_view text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(std::string_view word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy &, const Query &query, DocumentPredicate document_predicate) const
    {
        using namespace std;
        map<int, double> document_to_relevance;
        for (auto &&word : query.plus_words)
        {
            auto it = word_to_document_freqs_.find(word);
            if (it == word_to_document_freqs_.end())
            {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);

            for (const auto [document_id, term_freq] : it->second)
            {
                const auto &document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (auto &&word : query.minus_words)
        {
            auto it = word_to_document_freqs_.find(word);
            if (it == word_to_document_freqs_.end())
            {
                continue;
            }
            for (const auto [document_id, _] : it->second)
            {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::parallel_policy &, const Query &query, DocumentPredicate document_predicate) const;
};

template <typename ExPolicy>
void SearchServer::RemoveDocument(ExPolicy policy, int document_id)
{
    document_ids_.erase(document_id);
    documents_.erase(document_id);
    for (auto &[word, freq] : document_to_word_freqs[document_id])
    {
        word_to_document_freqs_[word].erase(document_id);
    }

    auto document_to_word_freqs_tmp = std::vector(document_to_word_freqs[document_id].begin(), document_to_word_freqs[document_id].end());
    std::for_each(policy, document_to_word_freqs_tmp.begin(), document_to_word_freqs_tmp.end(),
                  [document_id, this](auto &key_value)
                  {
                      auto &[word, freq] = key_value;
                      this->word_to_document_freqs_[word].erase(document_id);
                  });

    document_to_word_freqs.erase(document_id);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::parallel_policy &, const Query &query, DocumentPredicate document_predicate) const
{
    using namespace std;
    auto document_to_relevance = ConcurrentMap<int, double>{20};
    auto compute_relevance_kernel = [this, &document_to_relevance, &document_predicate](auto &&word)
    {
        auto it = word_to_document_freqs_.find(word);
        if (it == word_to_document_freqs_.end())
        {
            return;
        }
        const auto inverse_document_freq = ComputeWordInverseDocumentFreq(word);

        for (const auto [document_id, term_freq] : it->second)
        {
            const auto &document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating))
            {
                document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
            }
        }
    };

    for_each(std::execution::par, query.plus_words.begin(), query.plus_words.end(), compute_relevance_kernel);

    auto remove_minus_words_kernel = [this, &document_to_relevance](auto &&word)
    {
        auto it = word_to_document_freqs_.find(word);
        if (it == word_to_document_freqs_.end())
        {
            return;
        }
        for (const auto &[document_id, _] : it->second)
        {
            document_to_relevance.Erase(document_id);
        }
    };

    for_each(std::execution::par, query.minus_words.begin(), query.minus_words.end(), remove_minus_words_kernel);

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance.MergeTreesToVectorUnsafe())
    {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}