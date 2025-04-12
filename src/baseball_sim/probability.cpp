#include "probability.hpp"

#include <random>
#include <iostream>
#include <cassert>

std::mt19937 rand_gen;

void set_up_rand() {
    rand_gen = std::mt19937(time(NULL));
}


void calculate_event_probabilities(float x[], float y[], float z[], float output[], int num_events) {
    float total = 0;
    for (int i = 0; i < num_events; i++) {
        output[i] = x[i]*y[i]/z[i];
        total += output[i];
    }

    for (int i = 0; i < num_events; i++) {
        output[i] = output[i]/total;
    }
}


int get_random_event(float event_probs[], int num_events) {
    #if BASEBALL_DEBUG
        float sum = 0;
        for (int i = 0; i < num_events; sum+=event_probs[i], i++);
        if (abs(1 - sum) > 1e-6) {
            std::cout << "SUM: " << sum << "\n";
            for (int i = 0; i < num_events; i++) std::cout << "\t" << event_probs[i] << "\n";
            std::cout << "abs(1-sum) = " << abs(1 - sum) << "\n";
            assert(abs(1 - sum) < 1e-6);
        }
    #endif

    std::discrete_distribution<> dist(event_probs, event_probs + num_events);
    return dist(rand_gen);
}