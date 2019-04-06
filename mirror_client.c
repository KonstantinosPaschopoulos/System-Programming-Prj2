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
#include <sys/wait.h>
#include <sys/inotify.h>
#include "my_functions.h"

volatile sig_atomic_t flag = 1;

void catchinterrupt(int signo){
  flag = 0;
  printf("Caught %d. Deleting the mirror_dir and the .id file from the common_dir\n", signo);
}

int main(int argc, char **argv){
  pid_t deleter, sync;
  int inotifyFd, wd, length, read_ptr, read_offset, i, id, b, status;
  char id_str[100], buffStr[100], id_file[100], message[100], buffer[EVENT_BUF_LEN];
  char common_path[50], input_path[50], mirror_path[50], tmp_path[100], logfile_name[50];
  DIR *common_dir, *input_dir, *mirror_dir, *dir;
  FILE *logfile = NULL, *idfile = NULL;
  static struct sigaction act;
  struct dirent *ent;

  // Setting up the signal handler
  act.sa_handler = catchinterrupt;
  sigfillset(&(act.sa_mask));
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);

  // Parsing the input from the command line
  if (argc != 13)
  {
    printf("Usage: ./mirror_client -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l log_file\n");
    exit(1);
  }
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-n") == 0)
    {
      id = atoi(argv[i + 1]);
      i++;
    }
    else if (strcmp(argv[i], "-c") == 0)
    {
      // Creating the common_dir if it doesn't exist
      common_dir = opendir(argv[i + 1]);
      strcpy(common_path, argv[i + 1]);
      if (common_dir == NULL)
      {
        if (errno == ENOENT)
        {
          if (mkdir(argv[i + 1], 0777) == -1)
          {
            perror("mkdir failed");
            exit(2);
          }
          common_dir = opendir(argv[i + 1]);
        }
        else
        {
          perror("opendir failed");
          exit(2);
        }
      }

      i++;
    }
    else if (strcmp(argv[i], "-i") == 0)
    {
      // If the directory input_dir doesn't exist the program must terminate
      input_dir = opendir(argv[i + 1]);
      strcpy(input_path, argv[i + 1]);
      if (input_dir == NULL)
      {
        if (errno == ENOENT)
        {
          printf("input_dir does not exist\n");
          exit(1);
        }
        else
        {
          perror("opendir failed");
          exit(2);
        }
      }

      i++;
    }
    else if (strcmp(argv[i], "-m") == 0)
    {
      // If the directory mirror_dir exists the program must terminate
      mirror_dir = opendir(argv[i + 1]);
      strcpy(mirror_path, argv[i + 1]);
      if (mirror_dir)
      {
        if (closedir(mirror_dir) == -1)
        {
          perror("Closing the mirror directory failed");
          exit(2);
        }
        printf("mirror_dir already exists\n");
        exit(1);
      }
      else if (errno == ENOENT)
      {
        // Creating the directory if it doesn't exist
        if (mkdir(argv[i + 1], 0777) == -1)
        {
          perror("mkdir failed");
          exit(2);
        }
        mirror_dir = opendir(argv[i + 1]);
        if (closedir(mirror_dir) == -1)
        {
          perror("Closing the mirror directory failed");
          exit(2);
        }
      }
      else
      {
        perror("opendir failed");
        exit(2);
      }

      i++;
    }
    else if (strcmp(argv[i], "-b") == 0)
    {
      b = atoi(argv[i + 1]);
      if (b <= 0)
      {
        printf("b should be a positive number\n");
        exit(1);
      }
      sprintf(buffStr, "%d", b);

      i++;
    }
    else if (strcmp(argv[i], "-l") == 0)
    {
      // Check if the logfile exists already
      if (access(argv[i + 1], F_OK) != -1)
      {
        printf("The logfile already exists\n");
        exit(1);
      }

      strcpy(logfile_name, argv[i + 1]);
      logfile = fopen(argv[i + 1], "w");
      if (logfile == NULL)
      {
        perror("Couldn't create the logfile");
        exit(2);
      }
      fclose(logfile);

      i++;
    }
    else
    {
      printf("Usage: ./mirror_client -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l log_file\n");
      exit(1);
    }
  }

  sprintf(id_str, "%d", id);
  sprintf(tmp_path, "%s/%d.id", common_path, id);

  // Checking if the .id file exists already and is in use by a different client
  if (access(tmp_path, F_OK) != -1)
  {
    printf("The .id file already exists\n");
    exit(1);
  }

  // Creating the .id file in the common_dir and writing the pid on it
  idfile = fopen(tmp_path, "w");
  if (idfile == NULL)
  {
    perror("Couldn't open the .id file");
    exit(2);
  }
  fprintf(idfile, "%d", getpid());
  fclose(idfile);

  sprintf(message, "CLIENT_CONNECTED %d\n", id);
  write_to_logfile(logfile_name, message);

  // Doing the initial sync
  sprintf(id_file, "%d.id", id);
  dir = opendir(common_path);
  if (dir == NULL)
  {
    perror("Could not open input directory");
    exit(2);
  }
  while ((ent = readdir(dir)) != NULL)
  {
    if (ent->d_type == DT_DIR)
    {
      continue;
    }
    else
    {
      if ((strstr(ent->d_name, ".id") != NULL) && (strcmp(ent->d_name, id_file) != 0))
      {
        // Each syncing happens in a new process
        sync = fork();
        if (sync < 0)
        {
          perror("Syncing Fork Failed");
          exit(2);
        }
        if (sync == 0)
        {
          execl("./syncing", "syncing", common_path, id_str, ent->d_name, buffStr, input_path, logfile_name, mirror_path, NULL);
          perror("exec failed");
          exit(2);
        }
      }
    }
  }

  // Wait for the first syncing phase to finish
  while (wait(&status) > 0);

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

  read_offset = 0;
  while (flag == 1)
  {
    // Read next series of events
		length = read(inotifyFd, buffer + read_offset, sizeof(buffer) - read_offset);
		if (length < 0)
    {
      if (errno == EINTR)
      {
        // Read was interupted by a signal, check if it was SIGINT or SIGQUIT
        if (flag == 0)
        {
          break;
        }
        else
        {
          perror("inotify read was interupted by a signal different than SIGINT or SIGQUIT");
          exit(2);
        }
      }
      perror("inotify read failed");
      exit(2);
    }
		length += read_offset;
		read_ptr = 0;

    // Proccessing each event
    while ((int)(read_ptr + EVENT_SIZE) <= length )
    {
			struct inotify_event *event = (struct inotify_event *) &buffer[read_ptr];

			if ((int)(read_ptr + EVENT_SIZE + event->len) > length)
      {
        // In this case we cannot fully read all event data and need to wait until the next read
        break;
      }

			// In this case the event is fully received
      if (event->mask & IN_CREATE)
      {
        // A new .id file has been created
        if (strstr(event->name, ".id") != NULL)
        {
          // Each syncing happens in a new process
          sync = fork();
          if (sync < 0)
          {
            perror("Syncing Fork Failed");
            exit(2);
          }
          if (sync == 0)
          {
            execl("./syncing", "syncing", common_path, id_str, event->name, buffStr, input_path, logfile_name, mirror_path, NULL);
            perror("exec failed");
            exit(2);
          }
        }
      }
      else if (event->mask & IN_DELETE)
      {
        if ((strstr(event->name, ".id") != NULL) && (strcmp(event->name, id_file) != 0))
        {
          // The file that was deleted is an .id file
          deleter = fork();
          if (deleter < 0)
          {
            perror("Deleter Fork Failed");
            exit(2);
          }
          if (deleter == 0)
          {
            execl("./deleter", "deleter", mirror_path, event->name, NULL);
            perror("exec failed");
            exit(2);
          }

          // Wait for the removal to be completed to avoid race conditions
          waitpid(deleter, &status, 0);
          printf("Removal of %s from the mirror folder has been succesful.\n", event->name);
        }
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
  }

  while (wait(&status) > 0);

  // Closing the inotify instance
  inotify_rm_watch(inotifyFd, wd);
  close(inotifyFd);

  // Deleting the local mirror_dir
  deleter = fork();
  if (deleter < 0)
  {
    perror("rm mirror_dir Fork Failed");
    exit(2);
  }
  if (deleter == 0)
  {
    execl("/bin/rm", "rm", "-rf", mirror_path, NULL);
    perror("exec failed");
    exit(2);
  }
  wait(&status);

  // Deleting the .id file
  deleter = fork();
  if (deleter < 0)
  {
    perror("rm .id Fork Failed");
    exit(2);
  }
  if (deleter == 0)
  {
    execl("/bin/rm", "rm", "-rf", tmp_path, NULL);
    perror("exec failed");
    exit(2);
  }
  wait(&status);

  if (closedir(input_dir) == -1)
  {
    perror("Closing the input directory failed");
    exit(2);
  }
  if (closedir(dir) == -1)
  {
    perror("Closing the common directory failed");
    exit(2);
  }
  if (closedir(common_dir) == -1)
  {
    perror("Closing the common directory failed");
    exit(2);
  }

  sprintf(message, "CLIENT_DISCONNECTED %d\n", id);
  write_to_logfile(logfile_name, message);

  return 0;
}
