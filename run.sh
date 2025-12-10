#!/usr/bin/bash -x

SHARED="/tmp/shared"
DATA=$SHARED/data
RESULTS=$SHARED/results.txt

mkdir -p $SHARED

# create an empty results.txt file otherwise unikraft (still) fail when
# creating it
> $RESULTS

touch $SHARED/data

for MSIZE in 32 64 128 256 512 1024 2048 4096
do

  for i in {0..49}
  do
  dd if=/dev/urandom of=$DATA bs=64M count=16 iflag=fullblock

  printf "MSIZE: %s\t" $MSIZE >> $RESULTS
  qemu-system-x86_64 -cpu host --enable-kvm -nographic -m 1G \
    -nodefaults -serial stdio -kernel .unikraft/build/bob_qemu-x86_64 \
    -append 'vfs.fstab=[ "fs1:/:9pfs:::mkmp" ] -m $MSIZE --' \
    -virtfs local,path=$SHARED,mount_tag=fs1,security_model=passthrough

  done
done

mv $RESULTS compare_msize_1G.txt
