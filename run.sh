#!/usr/bin/bash -x

SHARED="/tmp/shared"
DATA=$SHARED/data
RESULTS=$SHARED/results.txt
OUTDIR="results"

mkdir -p $SHARED
mkdir -p $OUTDIR
rm ${OUTDIR}/*

for size in 1
do
  BUFSIZE=${size}"00000"
  printf "buffer size: %s\n" $BUFSIZE

# create an empty results file otherwise unikraft (still) fails when creating
# it
> $RESULTS

# for _ in {0..99}
# do
dd if=/dev/urandom of=$DATA bs=64M count=16 iflag=fullblock

qemu-system-x86_64 -cpu host --enable-kvm -nographic -m 1G \
  -nodefaults -serial stdio -kernel .unikraft/build/bob_qemu-x86_64 \
  -append "vfs.fstab=[ \"fs1:/:9pfs:::mkmp\" ] -- $BUFSIZE read" \
  -virtfs local,path=$SHARED,mount_tag=fs1,security_model=passthrough

# done

sed -i 's/\x00//g' $RESULTS
sed -i '/^\s*$/d' $RESULTS
sort -n $RESULTS -o $OUTDIR/read_1G_${size}00K

done
