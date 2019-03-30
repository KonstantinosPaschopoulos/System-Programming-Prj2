#include <stdio.h>
#include <stdlib.h>
#include "my_functions.h"

void write_to_logfile(char *log, char *message){
  FILE *logfile = NULL;

  logfile = fopen(log, "a");
  if (logfile == NULL)
  {
    perror("Couldn't append to the logfile");
    exit(2);
  }

  fprintf(logfile, "%s\n", message);

  fclose(logfile);
}
