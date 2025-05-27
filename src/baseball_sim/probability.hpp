#pragma once

#include "includes.hpp"

#include <random>

extern std::mt19937 rand_gen;

void set_up_rand();
void calculate_event_probabilities(const float x[], const float y[], const float z[], float output[], uint num_events);
int get_random_event(const float event_probs[], uint num_events);