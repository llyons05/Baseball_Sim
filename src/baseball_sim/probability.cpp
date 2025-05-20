#include "probability.hpp"

#include "includes.hpp"

#include <random>
#include <iostream>
#include <cassert>

std::mt19937 rand_gen;

void set_up_rand() {
    rand_gen = std::mt19937(time(NULL));
}


void calculate_event_probabilities(const float x[], const float y[], const float z[], float output[], int num_events) {
    float total = 0;
    for (int i = 0; i < num_events; i++) {
        output[i] = x[i]*y[i]/z[i];
        total += output[i];
    }

    for (int i = 0; i < num_events; i++) {
        output[i] = output[i]/total;
    }

    debug_line(
        if (total == 0) {
            std::cout << "WARNING: NAN PROBABILITY\n";
            std::cout << "\tNUM EVENTS: " << num_events << ", z[] PROBS: ";
            for (int i = 0; i < num_events; i++) std::cout << z[i] << " ";
            std::cout << "\n";
        }
    )
}


int get_random_event(const float event_probs[], int num_events) {
    debug_line(
        float sum = 0;
        for (int i = 0; i < num_events; sum+=event_probs[i], i++);
        if (abs(1 - sum) > 1e-6) {
            std::cout << "SUM: " << sum << "\n";
            for (int i = 0; i < num_events; i++) std::cout << "\t" << event_probs[i] << "\n";
            std::cout << "abs(1-sum) = " << abs(1 - sum) << "\n";
            assert(abs(1 - sum) < 1e-6);
        }
    )

    std::discrete_distribution<> dist(event_probs, event_probs + num_events);
    return dist(rand_gen);
}