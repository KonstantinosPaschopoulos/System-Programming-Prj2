#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Usage: mirror_dir, id2.id
int main(int argc, char **argv){
  char * id2;
  char tmp_path[300];
  pid_t rm_pid;
  int status;

  id2 = strtok(argv[2], ".");
  sprintf(tmp_path, "%s/%s", argv[1], id2);

  // Using exec to call rm -rf on the mirror_dir/id
  rm_pid = fork();
  if (rm_pid < 0)
  {
    perror("rm Fork Failed");
    exit(2);
  }
  if (rm_pid == 0)
  {
    execl("/bin/rm", "rm", "-rf", tmp_path, NULL);
    perror("exec failed");
    exit(2);
  }

  // Making sure rm worked correctly
  if (wait(&status) > 0)
  {
    if (WIFEXITED(status))
    {
      if (WEXITSTATUS(status) > 0)
      {
        printf("rm failed\n");
        exit(2);
      }

      return 12;
    }
    else
    {
      printf("rm failed\n");
      exit(2);
    }
  }
}
