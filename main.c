#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

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

  printf("Reading data from %s...\n", PATH);

  // start the clock
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    free(buf);
    close(fd);
    perror("clock_gettime () failed:");
  }

  // read data
  while ((res = read(fd, buf, buf_size))) {
    if (res == -1) {
      free(buf);
      close(fd);
      perror("read () failed:");
      exit(EXIT_FAILURE);
    }
  }

  // stop the clock
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    free(buf);
    close(fd);
    perror ("clock_gettime () failed:");
  }

  return compute_time_ms(&start, &end);
}

int main(int argc, char **argv) {

  int fd, res, length, buf_size, data_size;
  long time;
  char *buf, *str;

  printf("Let's bench some stuff!\n\n");

  // Read buffer size from call
  if (argc < 2) { exit(EXIT_FAILURE); }
  buf_size = atoi(argv[1]);
  if (buf_size == 0) { exit(EXIT_FAILURE); }

  // Allocate buffer
  buf = malloc(buf_size);
  if (buf == NULL) { exit(EXIT_FAILURE); }

  // Read data size if set on call
  if (argc > 3) {
    data_size = atoi(argv[3]);
    if (data_size == 0) { exit(EXIT_FAILURE); }
  }

  // Determine path depending on the mode that is given from call
  if (strcmp(argv[2], "read") == 0) {
    fd = open(PATH, O_RDONLY, 0777);
  }
  else if (strcmp(argv[2], "write") == 0) {
    fd = open(PATH, O_WRONLY | O_CREAT, 0777);
  }
  else { exit(EXIT_FAILURE); }

  if (fd == -1) {
    free(buf);
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  // Run benchmark according the mode given on call
  if (strcmp(argv[2], "read") == 0) {
    time = benchmark_read(fd, buf_size, buf);
  }
  else { exit(EXIT_FAILURE); }

  free(buf);
  close(fd);

  // compute length of the string representing benchmark timing
  length = snprintf(NULL, 0, "%ld\n", time);
  if (length < 0) {
    perror("snprintf() failed:");
    exit(EXIT_FAILURE);
  }
  str = malloc((1 + length) * sizeof(char));
  if (buf == NULL) { exit(EXIT_FAILURE); }

  // open result file for appending new benchmark result
  fd = open(RESULTS, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (fd == -1) {
    free(str);
    perror("open () failed:");
    exit(EXIT_FAILURE);
  };

  // prepare result string
  res = sprintf(str, "%ld\n", time);
  if (res < 0) {
    free(str);
    close(fd);
    perror("sprintf() failed:"); exit(EXIT_FAILURE);
  }

  // write new benchmark result in result file
  res = write(fd, str, length + 1);
  free(str);
  close(fd);
  if (res == -1) { perror("write() failed:"); exit(EXIT_FAILURE); }

  fflush(stdout);
  return 0;
}
