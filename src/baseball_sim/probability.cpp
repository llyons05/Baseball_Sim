#include "probability.hpp"

#include <random>
#include <iostream>

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
    std::discrete_distribution<> dist(event_probs, event_probs + num_events);
    return dist(rand_gen);
}