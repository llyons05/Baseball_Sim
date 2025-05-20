#pragma once

#include <random>

extern std::mt19937 rand_gen;

void set_up_rand();
void calculate_event_probabilities(const float x[], const float y[], const float z[], float output[], int num_events);
int get_random_event(const float event_probs[], int num_events);