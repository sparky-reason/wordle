#include "wordle.h"

#include <fstream>
#include <cstdlib>
#include <ctime>
#include <memory>

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
    map<string, unique_ptr<WordleStrategy>> strategies;
    strategies["simple"] = make_unique<SimpleWordleStrategy>(words);
    strategies["greedy"] = make_unique<GreedyWordleStrategy>(words);
    strategies["greedy_adv"] = make_unique<GreedyWordleStrategy>(words, true);

    srand(time(nullptr)); // use current time as seed for random generator

    cout << "Playing example game..." << endl;
    string true_word = words[rand() % words.size()];
    for (auto& named_strat : strategies) {
        cout << "--- " << named_strat.first << " ---" << endl;
        play_wordle(true_word, *(named_strat.second));      
        cout << endl;
    }

    // play some games and print sum of needed turns
    cout << "Playing multiple games..." << endl;
    map<string, unsigned int> turn_sums;
    const int N_GAMES = 10;
    for (int i = 0; i < N_GAMES; ++i) {
        cout << i << endl;
        string true_word = words[rand() % words.size()];
        for (auto& named_strat : strategies) {
            turn_sums[named_strat.first] += play_wordle(true_word, *(named_strat.second), 10, true);
        }
    }
    cout << "Total number of turn needed" << endl;
    for (auto& named_turns : turn_sums) {
        cout << named_turns.first << " : " << named_turns.second << endl;
    }
}