#!/usr/bin/bash

# Where (at least) all input data lives (from the PoV of the host)
SHARED="/tmp/shared"
DATA=$SHARED/data
OUTDIR="results"
MODE="read"
PLATFORM="uk"
DIR="."
RESULTS=$SHARED/results.txt
BUFSIZE=1000
DATASIZE=1000

mkdir -p $SHARED
mkdir -p $OUTDIR
> $RESULTS
> $DATA
pattern=$(base64 < /dev/urandom | head -c 101)
yes "$pattern" | head -c ${DATASIZE}MB  > ${DATA}


qemu-system-x86_64 -cpu host --enable-kvm -nographic -m 1G\
  -nodefaults -serial stdio -kernel out/bob_qemu-x86_64 \
  -append "vfs.fstab=[ \"fs1:/:virtiofs:::mkmp\" ] -- read $DIR $BUFSIZE $DATASIZE" \
  -virtfs local,path=$SHARED,mount_tag=fs1,security_model=passthrough \
| sed 's/^.*SeaBIOS/SeaBIOS/'
