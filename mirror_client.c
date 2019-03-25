#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char **argv){
  int i, id, b;
  DIR *common_dir, *input_dir, *mirror_dir;
  FILE *logfile = NULL;

  // Checking the input from the command line
  if (argc != 13)
  {
    printf("Correct use is: ./mirror_client -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l log_file\n");
    exit(1);
  }

  // Parsing the input
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
      if (common_dir)
      {
        closedir(common_dir);
      }
      else if (errno == ENOENT)
      {
        // Creating the directory if it doesn't exist
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

      i++;
    }
    else if (strcmp(argv[i], "-i") == 0)
    {
      // If the directory input_dir doesn't exist the program must terminate
      input_dir = opendir(argv[i + 1]);
      if (input_dir)
      {
        closedir(input_dir);
      }
      else if (errno == ENOENT)
      {
        printf("input_dir does not exist\n");
        exit(1);
      }
      else
      {
        perror("opendir failed");
        exit(2);
      }

      i++;
    }
    else if (strcmp(argv[i], "-m") == 0)
    {
      // If the directory mirror_dir exists the program must terminate
      mirror_dir = opendir(argv[i + 1]);
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

      i++;
    }
    else if (strcmp(argv[i], "-l") == 0)
    {
      if (access(argv[i + 1], F_OK) != -1)
      {
        printf("The logfile already exists\n");
        exit(1);
      }

      logfile = fopen(argv[i + 1], "a");
      if (logfile == NULL)
      {
        perror("Couldn't open logfile");
        exit(2);
      }

      i++;
    }
    else
    {
      printf("Correct use is: ./mirror_client -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l log_file\n");
      exit(1);
    }
  }

  fprintf(logfile, "Hola\n");

  fclose(logfile);

  return 0;
}
