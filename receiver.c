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
  char fifoName[100], path[300], tmp_path[300];
  char *fileName;
  int fifoFd, b, remaining, dir_or_not;
  char *buffer;
  char *tmp;
  FILE *fp;

  id2 = strtok(argv[3], ".");
  b = atoi(argv[4]);

  // Creating the mirror folder for the current id2
  sprintf(tmp_path, "%s/%s", argv[5], id2);
  if (mkdir(tmp_path, 0777) == -1)
  {
    perror("mirror/id mkdir failed");
    exit(2);
  }

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

    fileName = (char*)calloc(nameLength, sizeof(char));
    if (fileName == NULL)
    {
      perror("Calloc failed");
      exit(2);
    }

    // Reading the name of the file that is going to be transfered
    if (read(fifoFd, fileName, nameLength) == -1)
    {
      perror("Reading failed");
      exit(2);
    }

    // Reading if the file is a directory or not
    if (read(fifoFd, &dir_or_not, sizeof(dir_or_not)) == -1)
    {
      perror("Reading failed");
      exit(2);
    }

    tmp = strchr(fileName, '/');
    if (tmp != NULL)
    {
      tmp = tmp + 1;
      strcpy(fileName, tmp);
    }
    sprintf(path, "%s/%s/%s", argv[5], id2, fileName);

    // If the file that was sent is a directory I only create the directory
    // and skip the rest of the proccess
    if (dir_or_not == 1)
    {
      if (mkdir(path, 0777) == -1)
      {
        perror("Directory mkdir failed");
        exit(2);
      }

      continue;
    }

    // In this case a file is going to be transfered
    fp = fopen(path, "w");
    if (fp == NULL)
    {
      perror("Couldn't create the mirrored file");
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
    while (remaining > 0)
    {
      memset(buffer, 0, b);
      if (remaining > b)
      {
        if (read(fifoFd, buffer, b) == -1)
        {
          perror("Reading failed");
          exit(2);
        }

        fwrite(buffer, sizeof(char), b, fp);
      }
      else
      {
        if (read(fifoFd, buffer, remaining) == -1)
        {
          perror("Reading failed");
          exit(2);
        }

        fwrite(buffer, sizeof(char), remaining, fp);
      }

      printf("________ %s ________\n", buffer);

      remaining -= b;
    }

    free(fileName);
    fclose(fp);
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
