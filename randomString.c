// This program gets called from the script and is used to create random strings
// of argv[1] length

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv){
  const char charSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char *ret;
  int key, i, len;

  srandom(time(NULL) * getpid());

  // Getting a random length
  len = (random() % (atoi(argv[2]) - atoi(argv[1]) + 1)) + atoi(argv[1]);

  ret = (char*)calloc(len + 1, sizeof(char));
  if (ret == NULL)
  {
    perror("Calloc failed");
    exit(2);
  }

  for (i = 0; i < len; i++)
  {
    key = random() % (sizeof(charSet) - 1);
    ret[i] = charSet[key];
  }

  ret[len] = '\0';

  printf("%s\n", ret);

  free(ret);

  return 0;
}
