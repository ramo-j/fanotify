#ifndef NOTIFY_H
#define NOTIFY_H

#include <string>

// Initialise fanotify
int my_fanotify_init(char* dir);

// Destroy fanotify
void my_notify_destroy();

// Start monitoring
void my_fanotify_start();

// Looping to get events, thread function
void* my_fanotify_get_event(void* args);

// Tell the event watcher to stop
void my_fanotify_stop();

#endif
