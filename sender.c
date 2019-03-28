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

void traverseInput(int, char *, int);

// Usage: common_dir, id1, id2.id, buffer size, input_dir
int main(int argc, char **argv){
  char * id2;
  char fifoName[100];
  int fifoFd;

  id2 = strtok(argv[3], ".");

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

  fifoFd = open(fifoName, O_WRONLY);
  if (fifoFd == -1)
  {
    perror("Opening FIFO failed");
    exit(2);
  }

  traverseInput(fifoFd, argv[5], atoi(argv[4]));

  short nameLength = (short)00;
  if (write(fifoFd, &nameLength, sizeof(nameLength)) == -1)
  {
    perror("Write failed");
    exit(2);
  }

  if (close(fifoFd) == -1)
  {
    perror("Close failed");
    exit(2);
  }

  exit(0);
}

void traverseInput(int fifoFd, char *input, int b){
  DIR *dir;
  struct dirent *ent;
  char next_path[300];
  short nameLength;
  int fileLength;
  FILE* fp;

  dir = opendir(input);
  if (dir == NULL)
  {
    perror("Could not open input directory");
    exit(2);
  }

  while ((ent = readdir(dir)) != NULL)
  {
    sprintf(next_path, "%s/%s", input, ent->d_name); // TODO send the whole path

    if (ent->d_type == DT_DIR)
    {
      if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
      {
        continue;
      }

      // We need to go deeper
      traverseInput(fifoFd, next_path, b);
    }
    else
    {
      // Sending 2 bytes that declare the length of the file's name
      nameLength = (short)strlen(ent->d_name);
      if (write(fifoFd, &nameLength, sizeof(nameLength)) == -1)
      {
        perror("Write failed");
        exit(2);
      }

      // Sending the name of the file
      if (write(fifoFd, &(ent->d_name), nameLength) == -1)
      {
        perror("Write failed");
        exit(2);
      }

      // Calculating and sending the size of the file
      fp = fopen(next_path, "r");
      if (fp == NULL)
      {
        perror("Could not open file");
        exit(2);
      }
      fseek(fp, 0L, SEEK_END);
      fileLength = (int)ftell(fp);

      if (write(fifoFd, &fileLength, sizeof(fileLength)) == -1)
      {
        perror("Write failed");
        exit(2);
      }
      printf("HERE%d\n", fileLength);

      fseek(fp, 0L, SEEK_SET);

      // Sending the file in b sized chunks
      // while(fileLength > 0)
      // {
      //   if (write(fifoFd, &nameLength, sizeof(nameLength)) == -1)
      //   {
      //     perror("Write failed");
      //     exit(2);
      //   }
      //   fileLength -= b;
      // }

      fclose(fp);
    }
  }

  if (closedir(dir) == -1)
  {
    perror("Closing the input directory failed");
    exit(2);
  }

  return;
}