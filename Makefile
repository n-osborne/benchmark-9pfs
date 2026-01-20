CC = gcc
CFLAGS = -Wall

.PHONY: clean benchmarks

benchmarks: bob-linux bob_qemu-x86_64 run.sh
	# read one file of 1Go with 1000Ko buffer
	./run.sh read linux 1000 1000
	# read one file of 5Go with 1000Ko buffer
	./run.sh read linux 1000 5000
	# write one file of 1Go with 1000Ko buffer
	./run.sh write linux 1000 1000
	# write one file of 5Go with 1000Ko buffer
	./run.sh write linux 1000 5000
	# read 1000 files of 1Mo with 1000Ko buffer
	./run.sh multiple-read linux 1000 1 1000
	# write 1000 files of 1Mo with 1000Ko buffer
	./run.sh multiple-write linux 1000 1 1000
	# Unikraft + 9pfs
	# read one file of 1Go with 1000Ko buffer
	./run.sh read uk 1000 1000
	# read one file of 5Go with 1000Ko buffer
	./run.sh read uk 1000 5000
	# write one file of 1Go with 1000Ko buffer
	./run.sh write uk 1000 1000
	# write one file of 5Go with 1000Ko buffer
	./run.sh write uk 1000 5000
	# read 1000 files of 1Mo with 1000Ko buffer
	./run.sh multiple-read uk 1000 1 1000
	# write 1000 files of 1Mo with 1000Ko buffer
	./run.sh multiple-write uk 1000 1 1000

bob-linux: main.c
	$(CC) $< $(CFLAGS) -o bob-linux

bob_qemu-x86_64: main.c Kraftfile
	UK_CFLAGS="-std=gnu17" kraft build
	cp .unikraft/build/bob_qemu-x86_64 bob_qemu-x86_64

clean:
	rm bob-linux
	rm bob_qemu-x86_64
	kraft clean

