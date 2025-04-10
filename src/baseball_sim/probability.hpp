#pragma once

#include <random>

extern std::mt19937 rand_gen;

void set_up_rand();
void calculate_event_probabilities(float x[], float y[], float z[], float output[], int num_events);
int get_random_event(float event_probs[], int num_events);