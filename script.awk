#!/bin/awk -f

BEGIN {
  TOTAL=0;
  MSIZE4096=0;
  MSIZE2048=0;
  MSIZE1024=0;
  MSIZE512=0;
  MSIZE256=0;
  MSIZE128=0;
  MSIZE64=0;
  MSIZE32=0;
}
{
  if ($2 == 32) { MSIZE32 += $4; TOTAL += 1; }
  else if ($2 == 64) { MSIZE64 += $4; }
  else if ($2 == 128) { MSIZE128 += $4; }
  else if ($2 == 256) { MSIZE256 += $4; }
  else if ($2 == 512) { MSIZE512 += $4; }
  else if ($2 == 1024) { MSIZE1024 += $4; }
  else if ($2 == 2048) { MSIZE2048 += $4; }
  else if ($2 == 4096) { MSIZE4096 += $4; }
}
END {
  printf("%s %s\n", 32, MSIZE32 / TOTAL)
  printf("%s %s\n", 64, MSIZE64 / TOTAL)
  printf("%s %s\n", 128, MSIZE128 / TOTAL)
  printf("%s %s\n", 256, MSIZE256 / TOTAL)
  printf("%s %s\n", 512, MSIZE512 / TOTAL)
  printf("%s %s\n", 1024, MSIZE1024 / TOTAL)
  printf("%s %s\n", 2048, MSIZE2048 / TOTAL)
  printf("%s %s\n", 4096, MSIZE4096 / TOTAL)
}
