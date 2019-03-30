// Some variables that I use in the inotify section of the code
#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 50))

// The function that is used in order to write to the logfiles
void write_to_logfile(char *, char *);
