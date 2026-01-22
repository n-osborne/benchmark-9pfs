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

// FACTOR is used to multiply cli-provided buf_size for writes operations
// /!\ later FACTOR is used as divisor to compute write_size from buf_size
#ifndef FACTOR
#define FACTOR 2
#endif

#ifndef RESULTS
#define RESULTS "results.txt"
#endif

enum mode { READ, WRITE, MULTIPLE_READ, MULTIPLE_WRITE };

long benchmark_read(char *dir, char *buf, int buf_size);

long benchmark_multiple_read(char *dir, char *buf, int buf_size);

long benchmark_write(char *dir, char *buf, int buf_size, long data_size);

long benchmark_multiple_write(
    char *dir,
    char* buf,
    int buf_size,
    long data_size,
    int nb_files);

long compute_time_ms(struct timespec *start, struct timespec *end);

void fill_buffer(char *buf, int buf_size);

int main(int argc, char **argv) {

  int fd, res, length, buf_size, nb_files;
  long data_size;
  long time_ms;
  char *buf, *str, *dir, *results;
  enum mode mode;

  fd = 0;
  buf = NULL;
  str = NULL;
  results = NULL;

  printf("Let's bench some stuff!\n\n");

  if (argc < 4) { exit(EXIT_FAILURE); }

  // get the mode argument from cli
  if ((strncmp(argv[1], "read", 4)) == 0) {
    mode = READ;
  } else if ((strncmp(argv[1], "write", 5)) == 0) {
    mode = WRITE;
  } else if ((strncmp(argv[1], "multiple-read", 13)) == 0) {
    mode = MULTIPLE_READ;
  } else if ((strncmp(argv[1], "multiple-write", 14)) == 0) {
    mode = MULTIPLE_WRITE;
  } else { exit(EXIT_FAILURE); }

  // Read dir from call
  // TODO: sanitize string
  dir = argv[2];

  // Read buffer size from call
  buf_size = atoi(argv[3]);
  if (buf_size == 0) { goto free_ressources_and_exit; }
  buf_size <<= 10; // input is given in kilobytes!

  // In order to allow picking one slice of a big one-time-randomly-filled
  // buffer when benchmarking writes operations
  if (mode == WRITE || mode == MULTIPLE_WRITE) {
    buf_size = buf_size * FACTOR;
    srandom(time(NULL));
  }

  // Allocate buffer
  buf = malloc(buf_size);
  if (buf == NULL) {
    perror("malloc() failed:");
    goto free_ressources_and_exit;
  }

  // Read data size if set on call
  if (argc > 4) {
    data_size = atoi(argv[4]);
    data_size <<= 20; // input is given in megabytes!
    if (data_size == 0) { goto free_ressources_and_exit; }
  }

  // Read nb files if set on call
  if (argc > 5) {
    nb_files = atoi(argv[5]);
    if (nb_files == 0) { goto free_ressources_and_exit; }
  }

  // Run benchmark according to the mode given on call
  switch (mode) {
    case READ:
      time_ms = benchmark_read(dir, buf, buf_size);
      break;
    case WRITE:
      time_ms = benchmark_write(dir, buf, buf_size, data_size);
      break;
    case MULTIPLE_READ:
      time_ms = benchmark_multiple_read(dir, buf, buf_size);
      break;
    case MULTIPLE_WRITE:
      time_ms = benchmark_multiple_write(dir, buf, buf_size, data_size, nb_files);
      break;
    default:
      goto free_ressources_and_exit;
  }

  // compute length of the string representing benchmark timing
  length = snprintf(NULL, 0, "%ld\n", time_ms);
  if (length < 0) {
    perror("snprintf() failed:");
    goto free_ressources_and_exit;
  }
  str = malloc((1 + length) * sizeof(char));
  if (str == NULL) {
    perror("malloc() failed:");
    goto free_ressources_and_exit;
  }

  // prepare result path
  results = malloc(strlen(dir) + strlen(RESULTS) + 1);
  if (results == NULL) {
    perror("malloc() failed:");
    goto free_ressources_and_exit;
  }

  strcpy(results, dir);
  strcat(results, "/");
  strcpy(results, RESULTS);

  // open result file for appending new benchmark result
  fd = open(results, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (fd == -1) {
    perror("open () failed:");
    goto free_ressources_and_exit;
  };

  // prepare result string
  res = sprintf(str, "%ld\n", time_ms);
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
  if (fd > 0) close(fd);
  if (buf) free(buf);
  if (str) free(str);
  if (results) free(results);
  return 0;

free_ressources_and_exit:
  if (fd > 0) close(fd);
  if (buf) free(buf);
  if (str) free(str);
  if (results) free(results);
  exit(EXIT_FAILURE);
}

long compute_time_ms(struct timespec *start, struct timespec *end) {
  long sec, ns;

  sec = end->tv_sec - start->tv_sec;
  ns = end->tv_nsec - start->tv_nsec;

  return ns / 1000000L + sec * 1e3;
}

void fill_buffer(char *buf, int buf_size) {
  int i;

  for (i = 0; i < buf_size; i++) {
    buf[i] = (char) random();
  }
}

long benchmark_read(char *dir, char *buf, int buf_size) {
  int fd, res;
  struct timespec start, end;
  char *path;
  size_t len;

  fd = 0;
  path = 0;
  len = strlen(dir) + strlen(PATH) + 1;
  path = malloc(len);
  if (path == NULL) exit(EXIT_FAILURE);

  strcpy(path, dir);
  strcat(path, "/");
  strcat(path, PATH);

  printf("Reading data from %s...\n", path);

  // open file
  fd = open(path, O_RDONLY, 0777);
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
  fd = close(fd);
  if (fd == -1) {
    perror("close() failed:");
    goto free_ressources_and_exit;
  }

  // buf is freed in main
  if (path) free(path);

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  if (buf) free(buf);
  if (path) free(path);
  if (fd > 0) close(fd);
  exit(EXIT_FAILURE);
}

long benchmark_multiple_read(char *dir, char *buf, int buf_size) {
  int res, fd;
  DIR *dirp;
  struct dirent *d_entry;
  struct timespec start, end;

  fd = 0;

  printf("Reading multiple files from %s directory...\n", dir);

  dirp = opendir(dir);
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
      char* path = NULL;
      asprintf(&path, "%s/%s", dir, d_entry->d_name);
      fd = (open(path, O_RDONLY, 0777));
      if (fd == -1) {
        fprintf(stderr, "open(%s) failed:", path);
        goto free_ressources_and_exit;
      }

      // read data
      while ((res = read(fd, buf, buf_size))) {
        if (res == -1) {
          perror("read() failed:");
          goto free_ressources_and_exit;
        }
      }

      // close file
      fd = close(fd);
      if (fd == -1) {
        perror("close() failed:");
        goto free_ressources_and_exit;
      }
    }
  }
  // d_entry == NULL (out of loop condition)
  // fd is closed (loop invariant)
  // check errno for readdir failure
  if (errno) {
    perror("readdir() failed:");
    goto free_ressources_and_exit;
  }

  // stop the clock
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    perror ("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  if (buf) free(buf);
  if (fd > 0) close(fd);
  exit(EXIT_FAILURE);
}

long benchmark_write(char *dir, char *buf, int buf_size, long data_size) {
  int fd, res, write_size, idx;
  long count;
  struct timespec start, end;
  char *path;
  size_t len;

  fd = 0;
  path = 0;
  len = strlen(dir) + strlen(PATH) + 1;
  path = malloc(len);
  if (path == NULL) { goto free_ressources_and_exit; }

  strcpy(path, dir);
  strcat(path, "/");
  strcat(path, PATH);

  printf("Writing some data to %s...\n", path);

  // compute write_size
  write_size = buf_size / FACTOR;

  // fill buffer with random data
  fill_buffer(buf, buf_size);

  // open file
  fd = open(path, O_WRONLY | O_CREAT, 0777);
  if (fd == -1) {
    perror("open () failed:");
    goto free_ressources_and_exit;
  };

  count = 0;

  // start the clock
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    perror("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // write data to file
  while (count < data_size) {

    // pick a starting point in the large buffer
    // buf_size should be at least twice write_size
    // ie FACTOR should be at least 2
    idx = random() % (buf_size - write_size);

    res = write(fd, buf + idx, write_size);
    if (res == -1) {
      perror("write() failed");
      goto free_ressources_and_exit;
    }
    count += res;
  }

  fsync(fd); // ensure all is really written

  // stop the clock
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    perror ("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // close file
  fd = close(fd);
  if (fd == -1) {
    perror("close() failed");
    goto free_ressources_and_exit;
  }

  // buf is freed in main
  if (path) free(path);

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  if (buf) free(buf);
  if (path) free(path);
  if (fd > 0) close(fd);
  exit(EXIT_FAILURE);
}

long benchmark_multiple_write(
    char *dir,
    char* buf,
    int buf_size,
    long data_size,
    int nb_files)
{

  int res, fd, i, write_size, idx;
  struct timespec start, end;
  long count;
  char *path;

  fd = 0;
  path = malloc(strlen(dir) + strlen(PATH) + 1);
  if (path == NULL) {
    perror("malloc() failed");
    goto free_ressources_and_exit;
  }

  strcpy(path, dir);
  strcat(path, "/");
  strcat(path, PATH);

  printf("Writing multiple times to %s...\n", path);

  // compute write_size
  write_size = buf_size / FACTOR;

  // fill buffer with random data
  fill_buffer(buf, buf_size);

  // start the clock
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  if (res == -1) {
    perror("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  for (i = 0; i < nb_files; i++) {
    // open file
    fd = open(path, O_WRONLY | O_CREAT, 0777);
    if (fd == -1) {
      perror("open () failed:");
      goto free_ressources_and_exit;
    };

    count = 0;

    // write data to file
    while (count < data_size) {

      // pick a starting point in the large buffer
      // buf_size should be at least twice write_size
      // ie FACTOR should be at least 2
      idx = random() % (buf_size - write_size);

      res = write(fd, buf + idx, write_size);
      if (res == -1) {
        perror("write() failed");
        goto free_ressources_and_exit;
      }
      count += res;
    }

    fsync(fd); // ensure all is really written

    // close file
    fd = close(fd);
    if (fd == -1) {
      perror("close() failed:");
      goto free_ressources_and_exit;
    }
  }
  // fd is closed (loop invariant)
  // path has been written nb_files time with random data

  res = clock_gettime(CLOCK_MONOTONIC, &end);
  if (res == -1) {
    perror ("clock_gettime () failed:");
    goto free_ressources_and_exit;
  }

  // buf is freed in main
  if (path) free(path);

  return compute_time_ms(&start, &end);

free_ressources_and_exit:
  if (buf) free(buf);
  if (path) free(path);
  if (fd > 0) close(fd);
  exit(EXIT_FAILURE);
}
