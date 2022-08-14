#include "wordle.h"

#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <random>
#include <algorithm>
#include <execution>

#include <windows.h>

using namespace std;

const string WORD_LIST_PATH = "./data/deutsch.txt";

int main()
{
    cout << "Reading word list..." << endl;

    ifstream file(WORD_LIST_PATH, ios::in);
    set<string> word_set;
    string word;
    while (file >> word) {
        for (auto& c : word) c = toupper(c);

        if (word.length() != WORDLE_LENGTH)
            continue;

        bool bad_char = false;
        for (auto c : word) {
            if (!(c >= 'A' && c <= 'Z') && !(c == 'Ä' || c == 'Ö' || c == 'Ü')) {
                bad_char = true;
                break;
            }
        }
        if (bad_char) continue;

        word_set.insert(word);
    }
    vector<string> words(word_set.begin(), word_set.end());
    cout << "Number of unique words with " << WORDLE_LENGTH << " characters: " << words.size() << endl;

    // setup strategies
    typedef function<unique_ptr<WordleStrategy>()> WordleStrategyGenerator;
    map<string, WordleStrategyGenerator> strategies;
    map<string, string> initial_guesses;

    // pre-calculate initial guesses
    string initial_guess_simple = SimpleWordleStrategy(words).guess_word();
    string initial_guess_greedy = GreedyWordleStrategy(words).guess_word();
    string initial_guess_greedy_adv = GreedyWordleStrategy(words, nullopt, true).guess_word();

    strategies["simple"] = [&] { return make_unique<SimpleWordleStrategy>(words, initial_guess_simple); };
    strategies["greedy"] = [&] { return make_unique<GreedyWordleStrategy>(words, initial_guess_greedy); };
    strategies["greedy_adv"] = [&] { return make_unique<GreedyWordleStrategy>(words, initial_guess_greedy_adv, true); };

    srand(time(nullptr)); // use current time as seed for random generator

    cout << "Playing example game..." << endl;
    string test_word = words[rand() % words.size()];
    for (auto& named_strat : strategies) {
        cout << "--- " << named_strat.first << " ---" << endl;
        play_wordle(test_word, *named_strat.second());
        cout << endl;
    }

    // play some games and log number of needed turns to file
    cout << "Playing multiple games..." << endl;

    // setup output file
    ostringstream out_filename;
    out_filename << "./data/strategy_turns_" << WORDLE_LENGTH << ".csv";
    ofstream out_file(out_filename.str());
    out_file << "word";
    for (auto& named_strat : strategies) {
        out_file << '\t' << named_strat.first;
    }
    out_file << endl;

    // get sample of test words
    const unsigned int N_GAMES = 1024;
    vector<string> test_words;
    std::sample(words.begin(), words.end(), std::back_inserter(test_words),
        N_GAMES, std::mt19937{ std::random_device{}() });
    
    // run games on test words in parallel
    const unsigned int MAX_TURNS = 10;
    mutex mtx;
    for_each(execution::par , test_words.begin(), test_words.end(),
        [&](auto test_word) {
            vector<unsigned int> turns;
            for (auto& named_strat : strategies) {
                turns.push_back(play_wordle(test_word, *named_strat.second(), MAX_TURNS, true));
            }

            {
                lock_guard<mutex> lock(mtx);
                cout << '.';
                out_file << test_word;
                for (auto t : turns) {
                    out_file << '\t' << t;
                }
                out_file << endl;
            }
        });
}