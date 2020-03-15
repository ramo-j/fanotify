#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/fanotify.h>
#include <unistd.h>
#include "notify.h"
#include "results.h"

static pthread_t thread;

static int fd = 0;
static struct fanotify_event_metadata* metadata;
static struct fanotify_event_metadata buff[200];
static ssize_t path_len;
static char* path;
static char* procfd_path;
static bool keepGoing;

int my_fanotify_init(char* dir)
{
	keepGoing = true;
	path = new char[PATH_MAX];
	procfd_path = new char[PATH_MAX];
	fd = fanotify_init(FAN_CLASS_NOTIF | FAN_CLOEXEC, O_RDONLY | O_LARGEFILE);

	if (fd == -1)
	{
		perror("fanotify_init");
		exit(EXIT_FAILURE);
	}

	if (fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN | FAN_OPEN_EXEC, AT_FDCWD, dir) == -1)
	{
		perror("fanotify_mark");
		exit(EXIT_FAILURE);
	}

	return fd;
}

void my_fanotify_start()
{
	pthread_create(&thread, NULL, my_fanotify_get_event, NULL);
}

void* my_fanotify_get_event(void* args)
{
	ssize_t len;

	/* Read some events */
	while (keepGoing)
	{
		len = read(fd, (void *) &buff, sizeof(buff));
		if (len == -1 && errno != EAGAIN)
		{
			perror("read");
			exit(EXIT_FAILURE);
		}

		if (len <= 0)
		{
			return NULL;
		}

		metadata = buff;

		while (FAN_EVENT_OK(metadata, len))
		{
			/* Check that run-time and compile-time structures match */
			if (metadata->vers != FANOTIFY_METADATA_VERSION)
			{
				fprintf(stderr, "Mismatch of fanotify metadata version.\n");
				exit(EXIT_FAILURE);
			}

			/* metadata->fd contains either FAN_NOFD, indicating a
			   queue overflow, or a file descriptor (a nonnegative
			   integer). Here, we simply ignore queue overflow. */
			if (metadata->fd >= 0)
			{
				/* Retrieve and print pathname of the accessed file */
				snprintf(procfd_path, PATH_MAX * sizeof(char), "/proc/self/fd/%d", metadata->fd);
				path_len = readlink(procfd_path, path, PATH_MAX * sizeof(char) - 1);
				if (path_len == -1)
				{
					perror("readlink");
					exit(EXIT_FAILURE);
				}
				path[path_len] = '\0';
				results_add(path);

				/* Close the file descriptor of the event */
				close(metadata->fd);
			}

			/* Advance to next event */
			metadata = FAN_EVENT_NEXT(metadata, len);
		}
	}
	return NULL;
}

void my_fanotify_stop()
{
	keepGoing = false;
}

void my_notify_destroy()
{
	pthread_join(thread, NULL);
	close(fd);
	delete [] path;
	delete [] procfd_path;
}
