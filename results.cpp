#include <map>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include "results.h"

static bool keepGoing;
static pthread_mutex_t* lock;
static map<string, int>* results;
static pthread_t thread;
static struct resultsThreadArgs ta;

void results_init()
{
	keepGoing = true;
	results = new map<string, int>();
	lock = new pthread_mutex_t;
	if (pthread_mutex_init(lock, NULL))
		throw new std::bad_alloc;
	pthread_mutex_unlock(lock);

	ta.fd = STDOUT_FILENO;
	ta.sleepDuration = 3;

	pthread_create(&thread, NULL, results_print_thread, (void *) &ta);
}

void results_destroy()
{
	pthread_join(thread, NULL);
	delete results;
	pthread_mutex_destroy(lock);
	delete lock;
}

void results_add(string s)
{
	pthread_mutex_lock(lock);

	(*results)[s]++;

	pthread_mutex_unlock(lock);
}

void results_stop()
{
	keepGoing = false;
}

void* results_print_thread(void* args)
{
	while (keepGoing)
	{
		sleep(((resultsThreadArgs*)args)->sleepDuration);

		pthread_mutex_lock(lock);

		if (!results->size())
		{
			dprintf(((resultsThreadArgs*)args)->fd, "{\"files\":[]}\n");
			pthread_mutex_unlock(lock);
			continue;
		}
		
		std::map<string,int>::iterator lookAhead = results->begin();
		lookAhead++;

		dprintf(((resultsThreadArgs*)args)->fd, "{\"files\":[");
		for (std::map<string,int>::iterator it = results->begin(); it != results->end(); ++it)
		{
			dprintf(((resultsThreadArgs*)args)->fd, "{\"file\":\"%s\", \"count\": %d}", it->first.c_str(), it->second);
			if (lookAhead++ != results->end())
			{
				dprintf(((resultsThreadArgs*)args)->fd, ", ");
			}
		}
		dprintf(((resultsThreadArgs*)args)->fd, "]}\n");

		results->clear();	
		pthread_mutex_unlock(lock);
	}

	return NULL;
}
