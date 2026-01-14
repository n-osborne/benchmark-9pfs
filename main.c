#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

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

long benchmark_read(int buf_size, char* buf) {
  int fd, res;
  struct timespec start, end;

  printf("Reading data from %s...\n", PATH);

  // open file
  fd = open(PATH, O_RDONLY, 0777);
  if (fd == -1) {
    perror("open () failed:");
    goto free_ressources_and_exit;
  };

  // start the clock
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    perror("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // read data
  while ((res = read(fd, buf, buf_size))) {
    if (res == -1) {
      perror("read () failed:");
      goto free_ressources_and_exit;
    }
  }

  // stop the clock
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    perror ("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // close file
  close(fd);

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  free(buf);
  close(fd);
  exit(EXIT_FAILURE);
}

long benchmark_multiple_read(int buf_size, char* buf) {
  int res, fd;
  DIR *dirp;
  struct dirent *d_entry;
  struct timespec start, end;

  fd = 0;

  dirp = opendir(".");
  if (dirp == NULL) {
    perror("opendir() failed");
    goto free_ressources_and_exit;
  }

  // start the clock
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    perror("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // for readdir
  errno = 0;

  // traverse the directory
  while ((d_entry = readdir(dirp))) {

    // select data* files only
    if (strncmp(d_entry->d_name, "data", 4) == 0) {

      // open file
      fd = (open(d_entry->d_name, O_RDONLY, 0777));
      if (fd == -1) {
        perror("open() failed:");
        goto free_ressources_and_exit;
      }

      // read data
      while ((res = read(fd, buf, buf_size))) {
        if (res == -1) {
          perror("read() failed:");
          goto free_ressources_and_exit;
        }
      }
    }

    // close file
    if (close(fd) == -1) {
      perror("close() failed:");
      goto free_ressources_and_exit;
    }
  }
  // d_entry == NULL (out of loop condition)
  // fd is closed (loop invariant)

  // stop the clock
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    perror ("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // check errno for readdir failure
  if (errno) {
    perror("readdir() failed:");
    goto free_ressources_and_exit;
  }

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  free(buf);
  if (fd) close(fd);
  exit(EXIT_FAILURE);
}

long benchmark_write(int buf_size, int data_size, char* buf) {
  int i, fd, res, count;
  struct timespec start, end;

  printf("Writing some data to %s...\n", PATH);

  // open file
  fd = open(PATH, O_WRONLY | O_CREAT, 0777);
  if (fd == -1) {
    perror("open () failed:");
    goto free_ressources_and_exit;
  };

  // fill the buffer with data
  for (i = 0; i < buf_size; i++) {
    buf[i] = 'a';
  }

  count = 0;

  // start the clock
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    perror("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // write data to file
  while (count < data_size) {
    res = write(fd, buf, buf_size);
    if (res == -1) {
      perror("write() failed");
      goto free_ressources_and_exit;
    }
    count += res;
  }

  // stop the clock
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    perror ("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // close file
  close(fd);

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  free(buf);
  close(fd);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  int fd, res, length, buf_size, data_size;
  long time;
  char *buf, *str;

  fd = 0;
  buf = NULL;
  str = NULL;

  printf("Let's bench some stuff!\n\n");

  // Read buffer size from call
  if (argc < 2) { exit(EXIT_FAILURE); }
  buf_size = atoi(argv[1]);
  if (buf_size == 0) { goto free_ressources_and_exit; }

  // Allocate buffer
  buf = malloc(buf_size);
  if (buf == NULL) {
    perror("malloc() failed:");
    goto free_ressources_and_exit;
  }

  // Read data size if set on call
  if (argc > 3) {
    data_size = atoi(argv[3]);
    if (data_size == 0) { goto free_ressources_and_exit; }
  }

  // Run benchmark according the mode given on call
  if (strcmp(argv[2], "read") == 0) {
    time = benchmark_read(buf_size, buf);
  }
  else if (strcmp(argv[2], "write") == 0) {
    time = benchmark_write(buf_size, data_size, buf);
  }
  else if (strcmp(argv[2], "multiple-read") == 0) {
    time = benchmark_multiple_read(buf_size, buf);
  }
  else { goto free_ressources_and_exit; }

  // compute length of the string representing benchmark timing
  length = snprintf(NULL, 0, "%ld\n", time);
  if (length < 0) {
    perror("snprintf() failed:");
    goto free_ressources_and_exit;
  }
  str = malloc((1 + length) * sizeof(char));
  if (str == NULL) {
    perror("malloc() failed:");
    goto free_ressources_and_exit;
  }

  // open result file for appending new benchmark result
  fd = open(RESULTS, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (fd == -1) {
    perror("open () failed:");
    goto free_ressources_and_exit;
  };

  // prepare result string
  res = sprintf(str, "%ld\n", time);
  if (res < 0) {
    perror("sprintf() failed:");
    goto free_ressources_and_exit;
  }

  // write new benchmark result in result file
  res = write(fd, str, length + 1);
  if (res == -1) {
    perror("write() failed:");
    goto free_ressources_and_exit;
  }

  fflush(stdout);
  if (fd) close(fd);
  if (buf) free(buf);
  if (str) free(str);
  return 0;

free_ressources_and_exit:
  if (fd) close(fd);
  if (buf) free(buf);
  if (str) free(str);
  exit(EXIT_FAILURE);
}
