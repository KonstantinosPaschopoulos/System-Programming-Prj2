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

// Usage: common_dir, id1, id2.id, buffer size
int main(int argc, char **argv){
  char * id2;
  char fifoName[100];
  int fifoFd, b;

  id2 = strtok(argv[3], ".");
  b = atoi(argv[4]);

  sprintf(fifoName, "%s/%s_to_%s.fifo", argv[1], argv[2], id2);

  //Creating the pipe to communicate with the receiver process
  if (mkfifo(fifoName, 0666) == -1)
  {
    if (errno != EEXIST)
    {
      perror("Sender FIFO");
      exit(2);
    }
  }

  // fifoFd = open(fifoName, O_WRONLY);
  // if (fifoFd == -1)
  // {
  //   perror("Opening FIFO failed");
  //   exit(2);
  // }
  //
  //
  //
  // close(fifoFd);
  remove(fifoName);

  exit(0);
}
