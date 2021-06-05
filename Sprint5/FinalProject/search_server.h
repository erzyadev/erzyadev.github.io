
#pragma once

#include <string>
#include <set>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include "string_processing.h"
#include "document.h"
#include "log_duration.h"

constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer
{
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer &stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) // Extract non-empty stop words
    {
        using namespace std;
        if (auto it = std::find_if_not(stop_words_.begin(), stop_words_.end(), IsValidWord);
            it != stop_words_.end())
        {
            throw invalid_argument("Invalid word encountered: "s + *it);
        }
    }

    explicit SearchServer(const std::string &stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)) // Invoke delegating constructor
                                                        // from std::string container
    {
    }

    void AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentPredicate document_predicate) const
    {
        using namespace std;

        LOG_DURATION("Operation time");
        const auto query = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query, document_predicate);

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

    std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentStatus status) const
    {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating)
                                { return document_status == status; });
    }

    std::vector<Document> FindTopDocuments(const std::string &raw_query) const
    {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
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

    const std::map<std::string, double> &GetWordFrequencies(int document_id) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const;

    void RemoveDocument(int document_id);

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::map<std::string, double>> document_to_word_freqs;
    static const std::map<std::string, double> EMPTY_WORD_FREQ_MAP;
    static constexpr double TOLERANCE = 1e-6;

    bool IsStopWord(const std::string &word) const
    {
        return stop_words_.count(word) > 0;
    }

    static bool IsValidWord(const std::string &word);

    std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

    static int ComputeAverageRating(const std::vector<int> &ratings);

    struct QueryWord
    {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string &text) const;

    struct Query
    {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string &text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string &word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query &query, DocumentPredicate document_predicate) const
    {
        using namespace std;
        map<int, double> document_to_relevance;
        for (const std::string &word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
            {
                const auto &document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const std::string &word : query.minus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word))
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
};
