# README

This repository implements benchmarks for reading and writing files in a ramfs
in linux and in a unikraft unikernel running in qemu and sharing a ramfs via
9pfs.

# Instructions

The repository provides a Makefile to build the necessary binaries and run the
benchmarks.

Make sure you have [kraftkit](https://unikraft.org/docs/cli/install) installed.
Then run:

```sh
$ make benchmarks
```
