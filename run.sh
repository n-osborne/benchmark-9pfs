#!/usr/bin/bash

usage()
{
  set +x
  printf "usage: run.sh MODE PLATFORM BUFSIZE DATASIZE NBFILES\n"
  printf "\n"
  printf "runs 100 times the experiment on unikraft+9pfs or linux\n"
  printf "\n"
  printf "MODE    : either read, write, multiple-read or multiple-write\n"
  printf "PLATFORM: either uk or linux\n"
  printf "BUFSIZE : buffer size in Ko\n"
  printf "DATASIZE: size of the data to read/write in Mo\n"
  printf "NBFILES ; number of files to write for multiple read/write\n"
}

if [ $# -lt 3 ]
then
  usage
  exit
fi

set -x

SHARED="/tmp/shared"
DATA=$SHARED/data
RESULTS=$SHARED/results.txt
OUTDIR="results"
MODE=$1
PLATFORM=$2
DIR=""
case $PLATFORM in
  uk) DIR=".";;
  linux) DIR="/tmp/shared";;
  *) usage; exit;;
esac
BUFSIZE=$3
DATASIZE=$4
NBFILES=$5

mkdir -p $SHARED
mkdir -p $OUTDIR

printf "buffer size: %s\n" $BUFSIZE

# create an empty results file otherwise unikraft (still) fails when creating
# it
> $RESULTS

# same for data file in case of write mode
> $DATA

# run the experiment 100 times
for _ in {0..99}
do

# write data if in read mode
if [ "$MODE" = "read" ]
then
  dd if=/dev/urandom of=${DATA} bs=${DATASIZE}MB count=1 iflag=fullblock
fi

if [ "$MODE" = "multiple-read" ]
then
  for ((i=0; i<$NBFILES; i++))
  do
    dd if=/dev/urandom of="$DATA$i" bs=${DATASIZE}M count=1 iflag=fullblock
  done
fi

# running the unikernel with approriate arguments
case $PLATFORM in
  uk)
    qemu-system-x86_64 -cpu host --enable-kvm -nographic -m 1G\
      -nodefaults -serial stdio -kernel .unikraft/build/bob_qemu-x86_64 \
      -append "vfs.fstab=[ \"fs1:/:9pfs:::mkmp\" ] -- $MODE $DIR $BUFSIZE $DATASIZE $NBFILES" \
      -virtfs local,path=$SHARED,mount_tag=fs1,security_model=passthrough
    ;;
  linux)
    ./a.out $MODE $DIR $BUFSIZE $DATASIZE $NBFILES
    ;;
  *)
    usage
    exit
    ;;
esac

done

# cleanup, sort and copy results
sed -i 's/\x00//g' $RESULTS
sed -i '/^\s*$/d' $RESULTS
sort -n $RESULTS -o $OUTDIR/${PLATFORM}_${MODE}_${DATASIZE}M_${BUFSIZE}K_${NBFILES}
rm ${SHARED}/*
