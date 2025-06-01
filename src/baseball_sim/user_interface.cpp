#include "user_interface.hpp"

#include <string>
#include <vector>
#include <iostream>

using namespace std;


string get_user_choice(const string& prompt, const vector<string>& choices) {
    string input = "";
    bool is_valid_input = false;
    while (!is_valid_input) {
        cout << prompt;
        cin >> input;
        cout << "\n";
        for (const string& choice : choices) {
            if (input == choice) {
                is_valid_input = true;
                break;
            }
        }
    }
    return input;
}


string get_simulation_type() {
    return get_user_choice("Simulate two-team series or full season? (t/s): ", {"t", "s"});
}


void wait_for_user_input(const string& prompt) {
    string temp;
    cout << prompt;
    getline(cin, temp);
}