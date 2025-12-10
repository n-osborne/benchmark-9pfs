#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1000000
#endif

#ifndef PATH
#define PATH "data"
#endif

#ifndef RESULTS
#define RESULTS "results.txt"
#endif

long compute_time_ms(struct timespec *start, struct timespec *end) {
  long sec, ns;

  sec = end->tv_sec - start->tv_sec;
  ns = end->tv_nsec - start->tv_nsec;

  return ns / 1000000L + sec * 1e3;
}

int main () {

  int fd, res, length;
  long time;
  char *buf;
  char *str;
  struct timespec start, end;

  printf("Let's try some stuff!\n\n");

  buf = malloc(BUF_SIZE);

  fd = open(PATH, O_RDONLY, 0777);
  if (fd == -1) {
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  printf("Reading %s...\n", PATH);

  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) { perror("clock_gettime () failed:"); }

  while (res = read(fd, buf, BUF_SIZE)) {
    if (res == -1) {
      close(fd);
      perror("read () failed:");
      exit(EXIT_FAILURE);
    }
  }

  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) { perror ("clock_gettime () failed:"); }

  close(fd);

  fd = open(RESULTS, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (fd == -1) {
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  time = compute_time_ms(&start, &end);
  length = snprintf(NULL, 0, "TIME: %ld\n", time);
  str = malloc((1 + length) * sizeof(char));

  res = sprintf(str, "TIME: %ld\n", time);
  if (res < 0) { perror("sprintf() failed:"); exit(EXIT_FAILURE); }

  res = write(fd, str, length + 1);
  free(str);
  close(fd);
  if (res == -1) { perror("write() failed:"); exit(EXIT_FAILURE); }

  fflush(stdout);
  return 0;
}
