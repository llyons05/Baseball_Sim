#pragma once

#include <string>
#include <vector>
#include <iostream>

std::string get_user_choice(const std::string& prompt, const std::vector<std::string>& choices);
std::string get_simulation_type();

template <class T>
T get_user_input(const std::string& prompt) {
    T input;
    std::cout << prompt;
    std::cin >> input;
    std::cout << "\n";
    return input;
}