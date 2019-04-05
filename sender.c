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

void traverseInput(int, char *, int, char *);

// Usage: common_dir, id1, id2.id, buffer size, input_dir, logfile
int main(int argc, char **argv){
  char * id2;
  char fifoName[100], message[100];
  int fifoFd;
  short nameLength;
  ssize_t nread;

  id2 = strtok(argv[3], ".");
  sprintf(fifoName, "%s/%s_to_%s.fifo", argv[1], argv[2], id2);

  //Creating the pipe to communicate with the receiver process
  if (mkfifo(fifoName, 0666) == -1)
  {
    if (errno != EEXIST)
    {
      perror("Sender FIFO");
      kill(getppid(), SIGUSR1);
      exit(6);
    }
  }

  fifoFd = open(fifoName, O_WRONLY);
  if (fifoFd == -1)
  {
    perror("Opening FIFO failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  traverseInput(fifoFd, argv[5], atoi(argv[4]), argv[6]);

  // Sending 00 to signal that the transfer is done
  nameLength = (short)00;
  if ((nread = write(fifoFd, &nameLength, sizeof(nameLength))) == -1)
  {
    perror("Write failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }
  sprintf(message, "BYTES_SENT %d\n", (int)nread);
  write_to_logfile(argv[6], message);

  if (close(fifoFd) == -1)
  {
    perror("Close failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  return 12;
}

void traverseInput(int fifoFd, char *input, int b, char *log){
  DIR *dir;
  struct dirent *ent;
  char next_path[300], message[100];
  short nameLength;
  int fileLength, n, dir_or_not, total;
  FILE* fp = NULL;
  char *buffer;
  ssize_t nread;

  dir = opendir(input);
  if (dir == NULL)
  {
    perror("Could not open input directory");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  while ((ent = readdir(dir)) != NULL)
  {
    total = 0;
    sprintf(next_path, "%s/%s", input, ent->d_name);
    if (ent->d_type == DT_DIR)
    {
      if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
      {
        continue;
      }

      // Sending 2 bytes that declare the length of the directory's name
      nameLength = (short)strlen(next_path);
      if ((nread = write(fifoFd, &nameLength, sizeof(nameLength))) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      // Sending the name of the directory
      if ((nread = write(fifoFd, next_path, nameLength)) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      // Sending an integer that represents if it is a regular file or a directory
      dir_or_not = 1;
      if ((nread = write(fifoFd, &dir_or_not, sizeof(dir_or_not))) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      sprintf(message, "BYTES_SENT %d\n", total);
      write_to_logfile(log, message);

      // We need to go deeper
      traverseInput(fifoFd, next_path, b, log);
    }
    else
    {
      // Sending 2 bytes that declare the length of the file's name
      nameLength = (short)strlen(next_path);
      if ((nread = write(fifoFd, &nameLength, sizeof(nameLength))) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      // Sending the name of the file
      if ((nread = write(fifoFd, next_path, nameLength)) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      // Sending an integer that represents if it is a regular file or a directory
      dir_or_not = 0;
      if ((nread = write(fifoFd, &dir_or_not, sizeof(dir_or_not))) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      // Calculating and sending the size of the file
      fp = fopen(next_path, "r");
      if (fp == NULL)
      {
        perror("Could not open file");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      fseek(fp, 0L, SEEK_END);
      fileLength = (int)ftell(fp);

      if ((nread = write(fifoFd, &fileLength, sizeof(fileLength))) == -1)
      {
        perror("Write failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }
      total += nread;

      fseek(fp, 0L, SEEK_SET);

      buffer = (char*)calloc(b, sizeof(char));
      if (buffer == NULL)
      {
        perror("Calloc failed");
        kill(getppid(), SIGUSR1);
        exit(6);
      }

      // Using fgets to read b bytes and send them through the named pipe
      while ((n = fread(buffer, sizeof(char), b, fp)) > 0)
      {
        if ((nread = write(fifoFd, buffer, n)) == -1)
        {
          perror("Write failed");
          kill(getppid(), SIGUSR1);
          exit(6);
        }
        total += nread;

        memset(buffer, 0, b);
      }

      sprintf(message, "FILE_SENT %s\n", next_path);
      write_to_logfile(log, message);
      sprintf(message, "BYTES_SENT %d\n", total);
      write_to_logfile(log, message);

      fclose(fp);
      free(buffer);
    }
  }

  if (closedir(dir) == -1)
  {
    perror("Closing the input directory failed");
    kill(getppid(), SIGUSR1);
    exit(6);
  }

  return;
}
