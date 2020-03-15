#include <iostream>
#include <unistd.h>
#include "notify.h"
#include "results.h"
using namespace std;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s MOUNT\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	try
	{
		results_init(); // Start of our results printer, is a thread
		my_fanotify_init(argv[1]); // Initialise the fanotify watcher

		my_fanotify_start(); // Start the fanotify watcher, is a thread

		sleep(5); // Let the threads do their thing for a while

		my_fanotify_stop(); // Tell the watcher to stop
		my_notify_destroy(); // Clean up the watcher

		results_stop(); // Tell the results printer to stop
		results_destroy(); // Clean up the results printer
	}
	catch (const std::bad_alloc &)
	{
		cerr << "Memory allocation failure" << endl;
		return 1;
	}
	return 0;
}
