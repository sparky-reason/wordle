#include "wordle.h"

#include <windows.h>

using namespace std;

Feedback check(const std::string& guess_word, const std::string& true_word) {
    Feedback feedback = { WRONG_CHAR };
    bool true_char_used[WORDLE_LENGTH] = { false };

    // check for exact matches
    for (unsigned int i = 0; i < WORDLE_LENGTH; ++i) {
        if (guess_word[i] == true_word[i]) {
            feedback[i] = RIGHT;
            true_char_used[i] = true;
        }
    }

    // check for wrong positions
    for (unsigned int i = 0; i < WORDLE_LENGTH; ++i) {
        if (feedback[i] != RIGHT) {
            for (unsigned int j = 0; j < WORDLE_LENGTH; ++j) {
                if (!true_char_used[j] && guess_word[i] == true_word[j]) {
                    feedback[i] = WRONG_POS;
                    true_char_used[j] = true;
                    break;
                }
            }
        }
    }

    return feedback;
}

bool is_win(const Feedback& feedback) {
    for (unsigned int i = 0; i < WORDLE_LENGTH; ++i) {
        if (feedback[i] != RIGHT) return false;
    }
    return true;
}

void print_feedback(const std::string& word, const Feedback& feedback) {

    const int COLOR_WHITE = 7;
    const int COLOR_YELLOW = 6;
    const int COLOR_GREEN = 10;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for (unsigned int i = 0; i < WORDLE_LENGTH; ++i) {
        switch (feedback[i]) {
        case WRONG_CHAR:
            SetConsoleTextAttribute(hConsole, COLOR_WHITE); break;
        case WRONG_POS:
            SetConsoleTextAttribute(hConsole, COLOR_YELLOW); break;
        case RIGHT:
            SetConsoleTextAttribute(hConsole, COLOR_GREEN); break;
        }
        cout << word[i];
    }
    cout << endl;
    SetConsoleTextAttribute(hConsole, COLOR_WHITE);
}


SimpleWordleStrategy::SimpleWordleStrategy(const std::vector<std::string>& words) :
    words(words)
{
    copy(words.begin(), words.end(), back_inserter(consistent_words));
}

void SimpleWordleStrategy::process_feedback(const std::string& guessed_word, const Feedback& feedback) {
    // remove all words that are inconsistent with the feedback
    consistent_words.erase(
        remove_if(consistent_words.begin(), consistent_words.end(),
            [&](auto& word) { return check(guessed_word, word) != feedback; }
        ),
        consistent_words.end()
    );
}

std::string SimpleWordleStrategy::guess_word() const {
    return *(consistent_words.begin());
}

void SimpleWordleStrategy::reset()
{
    consistent_words.clear();
    copy(words.begin(), words.end(), back_inserter(consistent_words));
}


GreedyWordleStrategy::GreedyWordleStrategy(const std::vector<std::string>& words, bool adverserial /*= false*/)
    : SimpleWordleStrategy(words), adverserial(adverserial)
{}

std::string GreedyWordleStrategy::guess_word() const {
    if (consistent_words.size() == 1)
        return *consistent_words.begin();

    // compute word scores
    vector<float> scores;
    for (auto& word : words) {
        scores.push_back(score_word(word));
    }

    // return word with maximum score
    return words[distance(scores.begin(), max_element(scores.begin(), scores.end()))];
}

float GreedyWordleStrategy::score_word(const std::string& word) const {
    map<Feedback, unsigned int> feedback_counts;
    for (auto& consistent_word : consistent_words) {
        feedback_counts[check(word, consistent_word)]++;
    }

    vector<unsigned int> counts;
    std::transform(feedback_counts.begin(), feedback_counts.end(),
        std::back_inserter(counts),
        [](auto& feedback_count) { return feedback_count.second; });

    if (adverserial) {
        return -static_cast<float>(*max_element(counts.begin(), counts.end()));
    }
    else {
        float entropy = 0.0f;
        for (auto count : counts) {
            entropy -= count * log(count);
        }
        return entropy;
    }
}

unsigned int play_wordle(const std::string& true_word, WordleStrategy& strategy, unsigned int MAX_TURNS /*= 10*/, bool silent /*= false*/) {
    strategy.reset();
    if (!silent)
        cout << true_word << endl;
    for (unsigned int i_turn = 0; i_turn < MAX_TURNS; ++i_turn) {
        std::string guess_word = strategy.guess_word();
        Feedback feedback = check(guess_word, true_word);
        if (!silent)
            print_feedback(guess_word, check(guess_word, true_word));
        if (is_win(feedback))
            return i_turn;
        strategy.process_feedback(guess_word, feedback);
    }
    return MAX_TURNS;
}