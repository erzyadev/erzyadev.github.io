#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <numeric>
#include <vector>
#include <iostream>

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(string_view text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            words.push_back(word);
            word = "";
        }
        else
        {
            word += c;
        }
    }
    words.push_back(word);

    return words;
}

struct Document
{
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer
{
public:
    void SetStopWords(string_view text)
    {
        for (const string &word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, string_view document,
                     DocumentStatus status, const vector<int> &ratings)
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string &word : words)
        {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
                           DocumentData{
                               ComputeAverageRating(ratings),
                               status});
    }

    template <class DocumentFilter>
    vector<Document> FindTopDocuments(
        string_view raw_query, DocumentFilter document_filter) const
    {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_filter);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document &lhs, const Document &rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < EPSILON)
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

    vector<Document> FindTopDocuments(
        string_view raw_query,
        DocumentStatus status_filter = DocumentStatus::ACTUAL) const
    {
        return FindTopDocuments(raw_query,
                                [status_filter](int document_id,
                                                DocumentStatus status, int rating) {
                                    return status == status_filter;
                                });
    }

    int GetDocumentCount() const
    {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(
        string_view raw_query, int document_id) const
    {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string &word : query.plus_words)
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
        for (const string &word : query.minus_words)
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

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    static constexpr double EPSILON = 1e-6;

    bool IsStopWord(string_view word) const
    {
        return count(stop_words_.begin(), stop_words_.end(), word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(string_view text) const
    {
        vector<string> words;
        for (const string &word : SplitIntoWords(text))
        {
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int> &ratings)
    {
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord
    {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string_view text) const
    {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-')
        {
            is_minus = true;
            text = text.substr(1);
        }
        return {
            string{text},
            is_minus,
            IsStopWord(text)};
    }

    struct Query
    {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(string_view text) const
    {
        Query query;
        for (const string &word : SplitIntoWords(text))
        {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop)
            {
                if (query_word.is_minus)
                {
                    query.minus_words.insert(query_word.data);
                }
                else
                {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string &word) const
    {
        return log(GetDocumentCount() * 1.0 /
                   word_to_document_freqs_.at(word).size());
    }

    template <class DocumentFilter>
    vector<Document> FindAllDocuments(const Query &query, DocumentFilter filter_predicate) const
    {
        map<int, double> document_to_relevance;
        for (const string &word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
            {
                auto document_status = documents_.at(document_id).status;
                auto document_rating = documents_.at(document_id).rating;
                if (filter_predicate(document_id, document_status, document_rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string &word : query.minus_words)
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

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back({document_id,
                                         relevance,
                                         documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};

//--------------------------------------------------------------
//--------Container Output Operators----------------------------
template <typename V1, typename V2>
ostream &operator<<(ostream &output, pair<V1, V2> p)
{
    return output << p.first << ": " << p.second;
}

template <typename Container>
ostream &Print(ostream &output, const Container &c)
{

    if (c.begin() != c.end())
    {
        auto it = c.begin();
        output << *it;
        for (++it; it != c.end(); ++it)
        {
            output << ", " << *it;
        }
    }
    return output;
}

template <typename Value>
auto &operator<<(ostream &output, const vector<Value> &v)
{
    output << '[';
    Print(output, v);
    output << ']';
    return output;
}

template <typename Value>
auto &operator<<(ostream &output, const set<Value> &s)
{

    output << '{';
    Print(output, s);
    output << '}';
    return output;
}

template <typename Key, typename Value>
auto &operator<<(ostream &output, const map<Key, Value> &s)
{
    output << '{';
    Print(output, s);
    output << '}';
    return output;
}

//----------------------------------------------------
//-----------Testing Framework------------------------------

template <typename T, typename U>
void AssertEqualImpl(const T &t, const U &u, const string &t_str, const string &u_str, const string &file,
                     const string &func, unsigned line, const string &hint)
{
    if (t != u)
    {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty())
        {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string &expr_str, const string &file, const string &func, unsigned line,
                const string &hint)
{
    if (!value)
    {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty())
        {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Function>
void RunTestImpl(Function f, const string &function_name)
{
    /* Напишите недостающий код */
    f();
    cerr << function_name << " OK\n";
}

#define RUN_TEST(func) RunTestImpl(func, #func);

//-----------------------------------------------------
//------------Unit Tests-------------------------------
// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent()
{
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1UL);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

/*
Разместите код остальных тестов здесь
*/
void TestMinusWords()
{
    const int doc_id1 = 42, doc_id2 = 1984;
    const string content1 = "cat in the city"s;
    const string content2 = "dog in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        const auto found_docs_all = server.FindTopDocuments("dog cat city");
        ASSERT_EQUAL(found_docs_all.size(), 2UL);
        const auto found_docs_minus_cat = server.FindTopDocuments("dog -cat city");
        ASSERT_EQUAL(found_docs_minus_cat.size(), 1UL);
        const auto found_minus_city = server.FindTopDocuments("dog -city");
        ASSERT_EQUAL(found_minus_city.size(), 0UL);
    }
}

void TestMatching()
{
    const int doc_id1 = 42, doc_id2 = 1984;
    const string content1 = "cat in the city"s;
    const string content2 = "dog in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        server.SetStopWords("in the"s);

        auto response = server.MatchDocument("hello cat", doc_id1);
        ASSERT_EQUAL(get<0>(response), vector({"cat"s}));

        response = server.MatchDocument("hello city dog", doc_id2);
        auto words_returned = get<0>(response);
        sort(words_returned.begin(), words_returned.end());
        ASSERT_EQUAL(words_returned, vector({"city"s, "dog"s}));
    }

    //test matching with minus words
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        server.SetStopWords("in the"s);
        auto response = server.MatchDocument("-hello cat", doc_id1);
        ASSERT_EQUAL(get<0>(response).size(), 1UL);
        response = server.MatchDocument("hello -city dog", doc_id2);
        ASSERT_EQUAL(get<0>(response).size(), 0UL);
    }
}

void TestDocumentCout()
{
    const int doc_id1 = 42, doc_id2 = 1984;
    const string content1 = "cat in the city"s;
    const string content2 = "dog in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), 1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), 2);
    }
}

struct TestPrepareHelper
{
    static const vector<string> default_content;
    static const vector<int> default_ids;
    static const vector<DocumentStatus> default_status;
    static const vector<int> default_ratings;

    static void PrepareForTest(SearchServer &server,
                               const vector<string> &content = default_content,
                               const vector<int> &ids = default_ids,
                               const vector<DocumentStatus> &status = default_status,
                               const vector<int> ratings = default_ratings)
    {
        for (size_t i = 0; i < content.size(); ++i)
        {
            server.AddDocument(ids[i], content[i], status[i], ratings);
        }
    }
};

//carefully prepared handmade relevance sorting test
//with rare and frequent words, and different document lengths
const vector<string> TestPrepareHelper::default_content = {"rare city cat"s,
                                                           "rare city noncat"s,
                                                           "short frequent document"s,
                                                           "another longer frequent document"s,
                                                           "super long very frequent document"s,
                                                           "least relevant not even in search result frequent document"s};

const vector<int> TestPrepareHelper::default_ids = {0, 1, 2, 3, 4, 5};
const vector<DocumentStatus> TestPrepareHelper::default_status(6, DocumentStatus::ACTUAL);

const vector<int> TestPrepareHelper::default_ratings = {1, 2, 3};

void TestRelevanceSortedAndTopDocumentCount()
{
    SearchServer server;
    TestPrepareHelper::PrepareForTest(server);
    server.SetStopWords("in the"s);
    auto response = server.FindTopDocuments("rare frequent cat");
    ASSERT_EQUAL_HINT(response.size(), (size_t)MAX_RESULT_DOCUMENT_COUNT, "Incorrect search result size");
    for (size_t i = 0; i < TestPrepareHelper::default_ids.size(); ++i)
    {
        int doc_id = TestPrepareHelper::default_ids[i];
        ASSERT_EQUAL(response[i].id, doc_id);
    }
}
void TestRatingComputation()
{
    const string content = "rare city cat"s;

    {
        const vector<int> ratings0 = {1, 2, 3};
        SearchServer server;
        server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings0);
        auto response = server.FindTopDocuments("rare frequent cat");
        ASSERT_EQUAL(response[0].rating, 2);
    }
    //testing integer rounding
    {
        const vector<int> ratings1 = {3, 4, 3};
        SearchServer server;
        server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings1);
        auto response = server.FindTopDocuments("rare frequent cat");
        ASSERT_EQUAL_HINT(response[0].rating, 3, "Interger rounding failed");
    }
    //testing negative rating
    {
        const vector<int> ratings2 = {-10, 2, 3};
        SearchServer server;
        server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings2);
        auto response = server.FindTopDocuments("rare frequent cat");
        ASSERT_EQUAL_HINT(response[0].rating, -1, "Negative average rating");
    }
}

void TestRelevanceComputation()
{

    static constexpr double EPSILON = 1e-6; //same tolerance as SearchServer private constant;
    SearchServer server;

    TestPrepareHelper::PrepareForTest(server);

    server.SetStopWords("in the"s);
    auto response = server.FindTopDocuments("rare frequent cat");
    ASSERT(abs(response[0].relevance - 0.96345725) < EPSILON);
    ASSERT(abs(response[1].relevance - 0.36620410) < EPSILON);
    ASSERT(abs(response[2].relevance - 0.13515504) < EPSILON);
    ASSERT(abs(response[3].relevance - 0.10136628) < EPSILON);
    ASSERT(abs(response[4].relevance - 0.08109302) < EPSILON);
}

void TestStatusFilter()
{

    SearchServer server;
    const vector<DocumentStatus> status = {
        DocumentStatus::REMOVED,
        DocumentStatus::IRRELEVANT,
        DocumentStatus::IRRELEVANT,
        DocumentStatus::BANNED,
        DocumentStatus::BANNED,
        DocumentStatus::BANNED,
    };
    TestPrepareHelper::PrepareForTest(server, TestPrepareHelper::default_content, TestPrepareHelper::default_ids, status);
    auto query = "rare frequent cat"s;
    auto response = server.FindTopDocuments(query, DocumentStatus::ACTUAL);
    ASSERT_EQUAL(response.size(), 0UL);
    response = server.FindTopDocuments(query, DocumentStatus::REMOVED);
    ASSERT_EQUAL(response.size(), 1UL);
    response = server.FindTopDocuments(query, DocumentStatus::IRRELEVANT);
    ASSERT_EQUAL(response.size(), 2UL);
    response = server.FindTopDocuments(query, DocumentStatus::BANNED);
    ASSERT_EQUAL(response.size(), 3UL);
}

void TestPredicateFilter()
{
    SearchServer server;
    TestPrepareHelper::PrepareForTest(server);
    //select even doc_ids only
    auto even_filter = [](auto id, auto status, auto rating) { return id % 2 == 0; };
    const auto response = server.FindTopDocuments("rare frequent cat", even_filter);
    vector<int> response_ids;
    transform(response.begin(), response.end(), back_inserter(response_ids), [](auto &document) { return document.id; });
    sort(response_ids.begin(), response_ids.end());
    ASSERT_EQUAL_HINT(response_ids, vector({0, 2, 4}), "Even document ids selection");
}

//-------------------------------------------
//---------------Test Runner-----------------
void TestSearchServer()
{
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatching);
    RUN_TEST(TestDocumentCout);
    RUN_TEST(TestRelevanceSortedAndTopDocumentCount);
    RUN_TEST(TestRatingComputation);
    RUN_TEST(TestRelevanceComputation);
    RUN_TEST(TestStatusFilter);
    RUN_TEST(TestPredicateFilter);
}

int main()
{
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}