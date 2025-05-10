#pragma once

#include <string>
#include <vector>
#include <iostream>

std::string get_user_choice(const std::string& prompt, const std::vector<std::string>& choices);
std::string get_simulation_type();
void wait_for_user_input(const std::string& prompt);

template <class T>
T get_user_input(const std::string& prompt) {
    T input;
    while (true) {
        std::cout << prompt;
        std::cin >> input;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(100, '\n');
        }
        else {
            break;
        }
    }
    std::cout << "\n";
    return input;
}