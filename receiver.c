#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include "my_functions.h"

volatile sig_atomic_t done = 0;

void alarm_action(int signo){
  kill(getppid(), SIGUSR2);
  done++;
}

// In case of an error I use this function to free
// the resources that were used before exiting
void cleanup(int fifoFd, char *fifoName, char *buffer){
  if (done != 0)
  {
    close(fifoFd);
    remove(fifoName);
    free(buffer);
    exit(3);
  }
}

// Usage: common_dir, id1, id2.id, buffer size, mirror_dir, logfile
int main(int argc, char **argv){
  char * id2;
  short nameLength;
  char fifoName[100], path[300], tmp_path[300], message[100];
  int fifoFd, b, remaining, dir_or_not, fileLength, total;
  ssize_t nread;
  char *buffer, *tmp, *fileName;
  FILE *fp = NULL;
  static struct sigaction act;

  // Setting up the signal handler
  act.sa_handler = alarm_action;
  sigfillset(&(act.sa_mask));
  sigaction(SIGALRM, &act, NULL);

  id2 = strtok(argv[3], ".");
  b = atoi(argv[4]);

  // Creating the mirror folder for the current id2
  sprintf(tmp_path, "%s/%s", argv[5], id2);
  if (mkdir(tmp_path, 0777) == -1)
  {
    perror("mirror/id mkdir failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  buffer = (char*)calloc((b + 1), sizeof(char));
  if (buffer == NULL)
  {
    perror("calloc failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  sprintf(fifoName, "%s/%s_to_%s.fifo", argv[1], id2, argv[2]);

  //Creating the pipe to communicate with the receiver process
  if (mkfifo(fifoName, 0666) == -1)
  {
    if (errno != EEXIST)
    {
      perror("Receiver FIFO");
      kill(getppid(), SIGUSR1);
      exit(6);
    }
  }

  alarm(30);

  fifoFd = open(fifoName, O_RDONLY);
  if (fifoFd == -1)
  {
    perror("Opening FIFO failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  cleanup(fifoFd, fifoName, buffer);
  alarm(0);

  while(1)
  {
    total = 0;
    alarm(30);

    // Reading the two initial bytes
    if ((nread = read(fifoFd, &nameLength, sizeof(nameLength))) == -1)
    {
      perror("Reading failed");
      kill(getppid(), SIGUSR1);
      exit(6);
    }
    total += nread;

    cleanup(fifoFd, fifoName, buffer);
    // Reset and reinitiate the alarm for the next read
    alarm(0);

    // This is the signal that the transfer has finished
    if (nameLength == 00)
    {
      sprintf(message, "BYTES_RECEIVED %d\n", total);
      write_to_logfile(argv[6], message);
      break;
    }

    fileName = (char*)calloc((nameLength + 1), sizeof(char));
    if (fileName == NULL)
    {
      perror("Calloc failed");
      kill(getppid(), SIGUSR1);
      exit(6);
    }

    alarm(30);

    // Reading the name of the file that is going to be transfered
    if ((nread = read(fifoFd, fileName, nameLength)) == -1)
    {
      perror("Reading failed");
      kill(getppid(), SIGUSR1);
      exit(6);
    }
    total += nread;

    cleanup(fifoFd, fifoName, buffer);
    alarm(0);
    alarm(30);

    // Reading if the file is a directory or not
    if ((nread = read(fifoFd, &dir_or_not, sizeof(dir_or_not))) == -1)
    {
      perror("Reading failed");
      kill(getppid(), SIGUSR1);
      exit(6);
    }
    total += nread;

    cleanup(fifoFd, fifoName, buffer);
    alarm(0);

    tmp = strchr(fileName, '/');
    if (tmp != NULL)
    {
      memmove(fileName, tmp + 1, strlen(tmp));
    }
    sprintf(path, "%s/%s/%s", argv[5], id2, fileName);

    // If the file that was sent is a directory I only create the directory
    // and skip the rest of the proccess
    if (dir_or_not == 1)
    {
      if (mkdir(path, 0777) == -1)
      {
        perror("Directory mkdir failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      sprintf(message, "BYTES_RECEIVED %d\n", total);
      write_to_logfile(argv[6], message);

      free(fileName);
      continue;
    }

    // In this case a file is going to be transfered
    fp = fopen(path, "w");
    if (fp == NULL)
    {
      perror("Couldn't create the mirrored file");
      kill(getppid(), SIGUSR1);
      exit(6);
    }

    alarm(30);

    // Reading the length of the file
    if ((nread = read(fifoFd, &fileLength, sizeof(fileLength))) == -1)
    {
      perror("Reading failed");
      kill(getppid(), SIGUSR1);
      exit(6);
    }
    total += nread;

    cleanup(fifoFd, fifoName, buffer);
    alarm(0);

    // Reading the file using a buffer of b bytes
    remaining = fileLength;
    while (remaining > 0)
    {
      alarm(30);

      memset(buffer, 0, b);
      if (remaining > b)
      {
        if ((nread = read(fifoFd, buffer, b)) == -1)
        {
          perror("Reading failed");
          kill(getppid(), SIGUSR1);
          exit(6);
        }
      }
      else
      {
        if ((nread = read(fifoFd, buffer, remaining)) == -1)
        {
          perror("Reading failed");
          kill(getppid(), SIGUSR1);
          exit(6);
        }
      }
      fwrite(buffer, sizeof(char), nread, fp);
      total += nread;

      cleanup(fifoFd, fifoName, buffer);
      alarm(0);

      remaining -= nread;
    }

    // I count only the files that I receive, not the directories
    sprintf(message, "FILE_RECEIVED %s\n", path);
    write_to_logfile(argv[6], message);
    sprintf(message, "BYTES_RECEIVED %d\n", total);
    write_to_logfile(argv[6], message);

    free(fileName);
    fclose(fp);
  }

  if (close(fifoFd) == -1)
  {
    perror("Close failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }
  if (remove(fifoName) == -1) // The receiver is the only one that removes the pipe, to avoid errors
  {
    perror("Remove failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }
  free(buffer);

  return 12;
}
