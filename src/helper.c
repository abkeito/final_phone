#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

void die(const char *s)
{
  perror(s);
  exit(1);
}

void *read_stdin(void *arg)
{
  char buf[256];
  while (1)
  {
    int n = read(0, buf, sizeof(buf));
    if (n == -1)
    {
      die("read");
    }
  }
}

void log_message(const char *filename, const char *format, ...)
{
  va_list args;
  FILE *logfile = fopen(filename, "a");
  if (logfile == NULL)
  {
    perror("fopen");
    return;
  }

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  fprintf(logfile, "%04d-%02d-%02d %02d:%02d:%02d ",
          t->tm_year + 1900,
          t->tm_mon + 1,
          t->tm_mday,
          t->tm_hour,
          t->tm_min,
          t->tm_sec);

  va_start(args, format);
  vfprintf(logfile, format, args);
  va_end(args);

  fprintf(logfile, "\n");
  fclose(logfile);
}

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}