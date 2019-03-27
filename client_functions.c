// The inotify code is based on the lectures of professor Alex Dellis
// Source: http://cgi.di.uoa.gr/~ad/k22/index.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/inotify.h>
#include "client_functions.h"

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 50))

void monitoring(char *common_path){
  int inotifyFd, wd;
  int length, read_ptr, read_offset;
	char buffer[EVENT_BUF_LEN];

  // Creating an inotify instance to monitor the common_dir
  inotifyFd = inotify_init();
  if (inotifyFd < 0)
  {
    perror("inotify init failed");
    exit(2);
  }

  wd = inotify_add_watch(inotifyFd, common_path, IN_CREATE | IN_DELETE);
  if (wd == -1)
  {
    perror("inotify_add_watch failed");
    exit(2);
  }

  // The client is monitoring the common_dir periodically
  // cheking for changes in the .id files
  read_offset = 0;
  while(1)  // TODO add the signal flag
  {
    // Read next series of events
		length = read(inotifyFd, buffer + read_offset, sizeof(buffer) - read_offset);
		if (length < 0)
    {
      perror("inotify read failed");
      exit(2);
    }
		length += read_offset;
		read_ptr = 0;

    // Proccessing each event
    while ((read_ptr + EVENT_SIZE) <= length )
    {
			struct inotify_event *event = (struct inotify_event *) &buffer[read_ptr];

			if ((read_ptr + EVENT_SIZE + event->len) > length)
      {
        // In this case we cannot fully read all event data and need to wait until the next read
        break;
      }

			// In this case the event is fully received
      if (event->mask & IN_CREATE)
      {
        // A new file has been created
        printf("NEW %s\n", event->name);
      }
      else if (event->mask & IN_DELETE)
      {
        // A file has been deleted
        printf("DEL %s\n", event->name);
      }

			// Advance read_ptr to the beginning of the next event
			read_ptr += EVENT_SIZE + event->len;
		}

    // Check to see if a partial event remains at the end
		if(read_ptr < length)
    {
			// Copy the remaining bytes and signal the next read to begin immediatelly after them
			memcpy(buffer, buffer + read_ptr, length - read_ptr);
			read_offset = length - read_ptr;
		}
    else
    {
      read_offset = 0;
    }

    // T = 1
    sleep(2);
  }

  inotify_rm_watch(inotifyFd, wd);
  close(inotifyFd);
}
