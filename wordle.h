#pragma once


#include <string>
#include <vector>
#include <list>
#include <array>
#include <optional>

const int WORDLE_LENGTH = 5;  // wordle length hardcoded for performance reasons

enum CharFeedback : std::int8_t
{
    WRONG_CHAR,  // wrong character
    WRONG_POS,  // character present, but at wrong position
    RIGHT  // character at right position
};

typedef std::array<CharFeedback, WORDLE_LENGTH> Feedback;

Feedback check(const std::string& guess_word, const std::string& true_word);

bool is_win(const Feedback& feedback);

void print_feedback(const std::string& word, const Feedback& feedback);


// Abstract base class for wordle strategies
class WordleStrategy {
public:
    virtual void process_feedback(const std::string& guessed_word, const Feedback& feedback) = 0;
    virtual std::string guess_word() const = 0;
    virtual void reset() = 0;
};


// Simple wordle strategy that picks the first word consistent with the gathered feedback
class SimpleWordleStrategy : public WordleStrategy {
public:
    SimpleWordleStrategy(const std::vector<std::string>& words, std::optional<std::string> initial_guess=std::nullopt);

    virtual void process_feedback(const std::string& guessed_word, const Feedback& feedback);
    virtual std::string guess_word() const;
    virtual void reset();

protected:
    const std::vector<std::string>& words;   
    std::optional<std::string> initial_guess;
    std::vector<std::string> consistent_words;
};


// Wordle strategy that optimizes gained information in each turn
class GreedyWordleStrategy : public SimpleWordleStrategy {
public:
    GreedyWordleStrategy(const std::vector<std::string>& words, std::optional<std::string> initial_guess = std::nullopt, bool adverserial = false);

    virtual std::string guess_word() const;

    float score_word(const std::string& word) const;

protected:
    bool adverserial;
};


unsigned int play_wordle(const std::string& true_word, WordleStrategy& strategy, unsigned int MAX_TURNS = 10, bool silent = false);