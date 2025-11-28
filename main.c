#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifndef BUF_SIZE
#define BUF_SIZE 100
#endif

int main () {
  int fd, i;
  char buf[BUF_SIZE];
  char *path;
  struct dirent *pDirent;
  DIR *pDir;

  printf ("Let's try some stuff!\n\n");

  // if (getcwd (buf, BUF_SIZE) == NULL) { perror ("getcwd () failed:"); }
  // printf ("current working directory: %s\n", buf);

  pDir = opendir ("/");
  if (pDir == NULL) { perror ("opendir (\"/\") failed:"); }

  pDirent = readdir (pDir);
  while (pDirent != NULL) {
    printf ("%s\n", pDirent->d_name);
    pDirent = readdir (pDir);
  }

  printf ("%s\n", path);
  fd = open (path, O_RDONLY, 0777);
  if (fd == -1) {
    perror ("open () failed:");
    exit(EXIT_FAILURE);
  };

  while (i = read (fd, buf, BUF_SIZE)) {
    if (i == -1) {
      close (fd);
      perror ("read () failed:");
      exit(EXIT_FAILURE);
    }
    printf ("%s\n", buf);
  }

  close (fd);

  return 0;
}
