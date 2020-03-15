#ifndef RESULTS_H
#define RESULTS_H

#include <string>
using namespace std;

struct resultsThreadArgs {
	int fd;
	int sleepDuration;
};

// Create, destroy our map
void results_init();
void results_destroy();

// Add result
void results_add(string s);

// stop outputting results
void results_stop();

// outputting thread
void* results_print_thread(void* args);

#endif
