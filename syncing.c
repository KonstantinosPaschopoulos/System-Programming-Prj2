// Every time a new .id file is created the main mirror_client proccess
// executes this program. Here the two children that are meant to do the syncing are created.
// Also this is where the SIGUSR signals are handled.

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
#include "my_functions.h"

volatile sig_atomic_t retry = 1;

void readinterrupt(int signo){
  printf("Caught %d, because the receiver child couldn't read from the pipe\n", signo);
}

void childerror(int signo){
  retry++;
  printf("Caught %d, because there was an error in a child process.\n", signo);
  if (retry < 4)
  {
    printf("Attempting to retry the connection.\n");
  }
  else
  {
    printf("The connection is impossible, aborting.\n");
  }
}

// Usage: common, id, new file, buffer, input, logfile, mirror
int main(int argc, char **argv){
  pid_t sender, receiver, pid;
  int status;
  static struct sigaction act;
  char message[100];

  // Setting up the signal handler
  sigfillset(&(act.sa_mask));
  act.sa_flags = SA_RESTART;
  act.sa_handler = childerror;
  sigaction(SIGUSR1, &act, NULL);
  act.sa_handler = readinterrupt;
  sigaction(SIGUSR2, &act, NULL);

  do {
    // Two children are created to deal with the transfers
    sender = fork();
    if (sender < 0)
    {
      perror("Sender Fork Failed");
      exit(2);
    }
    if (sender == 0)
    {
      execl("./sender", "sender", argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL);
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
      execl("./receiver", "receiver", argv[1], argv[2], argv[3], argv[4], argv[7], argv[6], NULL);
      perror("exec failed");
      exit(2);
    }

    while ((pid = wait(&status)) > 0)
    {
      // Seeing how each child exited
      if (pid == receiver)
      {
        if (WIFEXITED(status))
        {
          // They return 12 if everything went normally
          if (WEXITSTATUS(status) == 12)
          {
            printf("Transfer from %s to %s.id is complete.\n", argv[3], argv[2]);
            retry = 1;
          }
          // Check if the receiver had to wait for 30 seconds to receive any input
          else if (WEXITSTATUS(status) == 3)
          {
            sprintf(message, "30_SEC_WAIT %s\n", argv[3]);
            write_to_logfile(argv[6], message);
            kill(sender, SIGKILL);
            retry = 1;
          }
          else
          {
            kill(sender, SIGKILL);
          }
        }
      }
      else
      {
        if (WIFEXITED(status))
        {
          // They return 12 if everything went normally
          if (WEXITSTATUS(status) == 12)
          {
            retry = 1;
            printf("Transfer from %s.id to %s is complete.\n", argv[2], argv[3]);
          }
          else
          {
            kill(receiver, SIGKILL);
          }
        }
      }
    }
  } while ((retry > 1) && (retry < 4));

  return 0;
}
