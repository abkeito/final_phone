#ifndef HELPER_H
#define HELPER_H

#define LOG_FILE_SERVER "log/server.log"
#define LOG_FILE_CLIENT "log/client.log"

void die(const char *s);
void *read_stdin(void *arg);
void log_message(const char *filename, const char *format, ...);
int min(int a, int b);

#endif
