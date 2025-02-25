#include "probability.hpp"

#include <random>
#include <iostream>

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
    const float eps = 1e-7;
    float r = ((float)rand())/RAND_MAX;
    for (int i = 0; i < num_events; i++) {
        if (r - eps <= event_probs[i]) {
            return i;
        }
        r -= event_probs[i];
    }

    std::cout << "PROBABILITY ERROR\n";
    std::quick_exit(1);
    return -1;
}