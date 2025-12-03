#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#ifndef BUF_SIZE
#define BUF_SIZE 100
#endif

#ifndef PATH
#define PATH "data"
#endif

#ifndef RESULTS
#define RESULTS "results.txt"
#endif

int main () {

  int fd, res, time, length, start, end;
  char buf[BUF_SIZE];
  char *str;
  struct timespec tspec;

  printf("Let's try some stuff!\n\n");

  fd = open(PATH, O_RDONLY, 0777);
  if (fd == -1) {
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  printf("Reading %s...\n", PATH);

  res = clock_gettime(CLOCK_MONOTONIC, &tspec);
  if (res == -1) { perror("clock_gettime () failed:"); }
  start = tspec.tv_nsec / 1000;

  while (res = read(fd, buf, BUF_SIZE)) {
    if (res == -1) {
      close(fd);
      perror("read () failed:");
      exit(EXIT_FAILURE);
    }
  }

  res = clock_gettime(CLOCK_MONOTONIC, &tspec);
  if (res == -1) { perror ("clock_gettime () failed:"); }
  end = tspec.tv_nsec / 1000;

  close(fd);

  fd = open(RESULTS, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (fd == -1) {
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  time = end - start;
  length = snprintf(NULL, 0, "%d\n", time);
  str = malloc((1 + length) * sizeof(char));

  res = sprintf(str, "%d\n", time);
  if (res < 0) { perror("sprintf() failed:"); exit(EXIT_FAILURE); }

  res = write(fd, str, length + 1);
  free(str);
  close(fd);
  if (res == -1) { perror("write() failed:"); exit(EXIT_FAILURE); }

  fflush(stdout);
  return 0;
}
