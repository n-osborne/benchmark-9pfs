#!/usr/bin/bash

usage()
{
  printf "usage: run.sh BUFSIZE MODE DATASIZE\n"
  printf "\n"
  printf "runs 100 times the experiment\n"
  printf "\n"
  printf "BUFSIZE : buffer size in Ko\n"
  printf "MODE    : either read or write\n"
  printf "DATASIZE: size of the data to read/write in Mo\n"
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
BUFSIZE=$1
MODE=$2
DATASIZE=$3

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
  dd if=/dev/urandom of=$DATA bs=${DATASIZE}M count=1 iflag=fullblock
fi

# running the unikernel with approriate arguments
qemu-system-x86_64 -cpu host --enable-kvm -nographic -m 1G\
  -nodefaults -serial stdio -kernel .unikraft/build/bob_qemu-x86_64 \
  -append "vfs.fstab=[ \"fs1:/:9pfs:::mkmp\" ] -- $BUFSIZE $MODE $DATASIZE" \
  -virtfs local,path=$SHARED,mount_tag=fs1,security_model=passthrough

done

# cleanup, sort and copy results
sed -i 's/\x00//g' $RESULTS
sed -i '/^\s*$/d' $RESULTS
sort -n $RESULTS -o $OUTDIR/${MODE}_${DATASIZE}M_${BUFSIZE}K
