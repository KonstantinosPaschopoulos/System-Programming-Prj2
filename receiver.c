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

// Usage: common_dir, id1, id2.id, buffer size, mirror_dir
int main(int argc, char **argv){
  char * id2;
  short nameLength;
  int fileLength;
  char fifoName[100], fileName[100];
  int fifoFd, b, remaining;
  char *buffer;

  id2 = strtok(argv[3], ".");
  b = atoi(argv[4]);

  buffer = (char*)malloc(b * sizeof(char));
  if (buffer == NULL)
  {
    perror("Malloc failed");
    exit(2);
  }

  sprintf(fifoName, "%s/%s_to_%s.fifo", argv[1], id2, argv[2]);

  //Creating the pipe to communicate with the receiver process
  if (mkfifo(fifoName, 0666) == -1)
  {
    if (errno != EEXIST)
    {
      perror("Receiver FIFO");
      exit(2);
    }
  }

  fifoFd = open(fifoName, O_RDONLY);
  if (fifoFd == -1)
  {
    perror("Opening FIFO failed");
    exit(2);
  }

  while(1)
  {
    // Reading the two initial bytes
    if (read(fifoFd, &nameLength, sizeof(nameLength)) == -1)
    {
      perror("Reading failed");
      exit(2);
    }

    // This is the signal that the transfer has finished
    if (nameLength == 00)
    {
      break;
    }

    // Reading the name of the file that is going to be transfered
    if (read(fifoFd, &fileName, nameLength) == -1)
    {
      perror("Reading failed");
      exit(2);
    }

    // Reading the length of the file
    if (read(fifoFd, &fileLength, sizeof(fileLength)) == -1)
    {
      perror("Reading failed");
      exit(2);
    }

    // Reading the file using a buffer of b bytes
    remaining = fileLength;
    printf("THERE %s %d\n", fileName, fileLength);
    while (remaining > 0)
    {
      if (remaining > b)
      {
        if (read(fifoFd, buffer, b) == -1)
        {
          perror("Reading failed");
          exit(2);
        }
      }
      else
      {
        if (read(fifoFd, buffer, remaining) == -1)
        {
          perror("Reading failed");
          exit(2);
        }
      }

      printf("BUFFER %s\n", buffer);

      remaining -= b;
    }
  }

  if (close(fifoFd) == -1)
  {
    perror("Close failed");
    exit(2);
  }
  if (remove(fifoName) == -1) // The receiver is the only one that removes the pipe, to avoid errors
  {
    perror("Remove failed");
    exit(2);
  }
  free(buffer);

  exit(0);
}
