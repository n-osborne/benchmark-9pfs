#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

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

long benchmark_read(int fd, int buf_size, char* buf) {
  int res;
  struct timespec start, end;

  printf("Reading %s...\n", PATH);

  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    free(buf);
    close(fd);
    perror("clock_gettime () failed:");
  }

  while (res = read(fd, buf, buf_size)) {
    if (res == -1) {
      free(buf);
      close(fd);
      perror("read () failed:");
      exit(EXIT_FAILURE);
    }
  }

  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    free(buf);
    close(fd);
    perror ("clock_gettime () failed:");
  }

  free(buf);
  close(fd);

  return compute_time_ms(&start, &end);
}

int main(int argc, char **argv) {

  int fd, res, length, buf_size;
  long time;
  char *buf, *str;
  struct timespec start, end;

  printf("Let's try some stuff!\n\n");

  // Not the best error handling...
  if (argc < 1) { exit(EXIT_FAILURE); }
  buf_size = atoi(argv[1]);
  if (buf_size == 0) { exit(EXIT_FAILURE); }

  buf = malloc(buf_size);
  if (buf == NULL) { exit(EXIT_FAILURE); }

  fd = open(PATH, O_RDONLY, 0777);
  if (fd == -1) {
    free(buf);
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  time = benchmark_read(fd, buf_size, buf);
  length = snprintf(NULL, 0, "%ld\n", time);
  if (length < 0) {
    perror("snprintf() failed:");
    exit(EXIT_FAILURE);
  }
  str = malloc((1 + length) * sizeof(char));
  if (buf == NULL) { exit(EXIT_FAILURE); }

  fd = open(RESULTS, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (fd == -1) {
    free(str);
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  res = sprintf(str, "%ld\n", time);
  if (res < 0) {
    free(str);
    close(fd);
    perror("sprintf() failed:"); exit(EXIT_FAILURE);
  }

  res = write(fd, str, length + 1);
  free(str);
  close(fd);
  if (res == -1) { perror("write() failed:"); exit(EXIT_FAILURE); }

  fflush(stdout);
  return 0;
}
