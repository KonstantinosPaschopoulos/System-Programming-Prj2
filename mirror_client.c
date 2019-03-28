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

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 50))

volatile sig_atomic_t flag = 1;

void catchinterrupt(int signo){
  flag = 0;
  printf("Caught %d\n", signo);
}

int main(int argc, char **argv){
  pid_t sender, receiver;
  int inotifyFd, wd, length, read_ptr, read_offset, i, id, b, status;
	char buffer[EVENT_BUF_LEN];
  DIR *common_dir, *input_dir, *mirror_dir;
  char id_str[100], buffStr[100], id_file[100];
  char common_path[50], input_path[50], mirror_path[50], tmp_path[100];
  FILE *logfile = NULL, *idfile = NULL;
  static struct sigaction act;
  DIR *dir;
  struct dirent *ent;

  // Setting up the signal handler
  act.sa_handler = catchinterrupt;
  sigfillset(&(act.sa_mask));
  // TODO add signals
  // sigaction(SIGINT, &act, NULL);
  // sigaction(SIGQUIT, &act, NULL);

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
        closedir(mirror_dir);
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
      // TODO uncomment the following
      // if (access(argv[i + 1], F_OK) != -1)
      // {
      //   printf("The logfile already exists\n");
      //   exit(1);
      // }

      logfile = fopen(argv[i + 1], "w");
      if (logfile == NULL)
      {
        perror("Couldn't create the logfile");
        exit(2);
      }

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
      if ((strstr(ent->d_name, ".id") != NULL) && strcmp(ent->d_name, id_file) != 0)
      {
        sender = fork();
        if (sender < 0)
        {
          perror("Sender Fork Failed");
          exit(2);
        }
        if (sender == 0)
        {
          execl("sender", "sender", common_path, id_str, ent->d_name, buffStr, input_path, NULL);
          perror("exec failed");
          exit(2);
        }

        receiver = fork();
        if (receiver < 0)
        {
          perror("Receiver Fork Failed");
          exit(2);
        }
        if (receiver == 0)
        {
          execl("receiver", "receiver", common_path, id_str, ent->d_name, buffStr, mirror_path, NULL);
          perror("exec failed");
          exit(2);
        }

        wait(&status);
        wait(&status);
      }
    }
  }

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
  while(1)  // TODO add signal flag
  {
    printf("WAITING FOR CLIENTS\n");
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
        if (strstr(event->name, ".id") != NULL)
        {
          // The new file that was created is an .id file so the two children are created
          sender = fork();
          if (sender < 0)
          {
            perror("Sender Fork Failed");
            exit(2);
          }
          if (sender == 0)
          {
            execl("sender", "sender", common_path, id_str, event->name, buffStr, input_path, NULL);
            perror("exec failed");
            exit(2);
          }

          receiver = fork();
          if (receiver < 0)
          {
            perror("Receiver Fork Failed");
            exit(2);
          }
          if (receiver == 0)
          {
            execl("receiver", "receiver", common_path, id_str, event->name, buffStr, mirror_path, NULL);
            perror("exec failed");
            exit(2);
          }

          wait(&status);
          wait(&status);
        }
      }
      else if (event->mask & IN_DELETE)
      {
        // A file has been deleted
        if (strstr(event->name, ".id") != NULL)
        {
          printf("DEL %s\n", event->name);
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

    // T = 2
    sleep(2);
  }

  // Closing the inotify instance
  inotify_rm_watch(inotifyFd, wd);
  close(inotifyFd);

  if (closedir(mirror_dir) == -1)
  {
    perror("Closing the mirror directory failed");
    exit(2);
  }
  if (closedir(input_dir) == -1)
  {
    perror("Closing the input directory failed");
    exit(2);
  }
  if (closedir(common_dir) == -1)
  {
    perror("Closing the common directory failed");
    exit(2);
  }
  if (closedir(dir) == -1)
  {
    perror("Closing the common directory failed");
    exit(2);
  }
  fclose(logfile);

  return 0;
}
