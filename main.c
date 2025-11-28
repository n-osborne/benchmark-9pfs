#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifndef BUF_SIZE
#define BUF_SIZE 100
#endif

int main () {
  int fd ;
  char buf[BUF_SIZE];
  // struct dirent *pDirent;
  // DIR *pDir;

  printf ("Let's try some stuff!\n\n");

  // if (getcwd (buf, BUF_SIZE) == NULL) { perror ("getcwd () failed:"); }
  // printf ("current working directory: %s\n", buf);

  // pDir = opendir ("/");
  // if (pDir == NULL) { perror ("opendir (\"/\") failed:"); }

  // pDirent = readdir (pDir);
  // while (pDirent != NULL) {
  //   printf ("%s\n", pDirent->d_name);
  //   pDirent = readdir (pDir);
  // }

  fd = open ("./data", O_RDONLY, 0777);
  if (fd == -1) { perror ("open (\"./data\") failed:"); };

  if (0 > read (fd, buf, BUF_SIZE)) { close (fd); perror ("read () failed:"); }
  printf ("%s\n", buf);

  close (fd);

  return 0;
}
