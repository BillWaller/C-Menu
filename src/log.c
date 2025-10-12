/* log.c */

#include "menu.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_LOG_LINE 1023
int log_fd = -1;
char log_filename[] = "app.log";

void get_rfc3339_s(char *, size_t);
int open_log(char *);
void write_log(char *);

void get_rfc3339_s(char *rfc3339_p, size_t s_len) {
  time_t now;
  struct tm *tms;
  char timestr1[128];
  char timestr2[128];
  int i = 0, j;

  time(&now);
  tms = localtime((time_t *)&now);
  strftime(timestr1, sizeof(timestr1), "%z", tms);
  for (j = 0; j < 6; j++) {
    if (i == 3)
      timestr2[i++] = ':';
    timestr2[i++] = timestr1[j];
  }
  timestr2[6] = '\0';
  sprintf(rfc3339_p, "%04d-%02d-%02dT%02d:%02d:%02d%s", tms->tm_year + 1900,
          tms->tm_mon + 1, tms->tm_mday, tms->tm_hour, tms->tm_min, tms->tm_sec,
          timestr2);
}

int open_log(char *filename) {
  return (open(filename, O_CREAT | O_WRONLY,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
}

void write_log(char *text) {

  if (f_debug == FALSE)
    return;
  char rfc3339_s[30];
  size_t rfc3339_s_len;
  char log_line[MAX_LOG_LINE + 1];
  size_t log_line_len;

  if (log_fd == -1)
    log_fd = open_log(log_filename);
  rfc3339_s_len = sizeof(rfc3339_s);
  get_rfc3339_s(rfc3339_s, rfc3339_s_len);
  strcpy(log_line, rfc3339_s);
  strcat(log_line, ";");
  strcat(log_line, text);
  strcat(log_line, "\n");
  log_line_len = strlen(log_line);
  write(log_fd, log_line, log_line_len);
}
