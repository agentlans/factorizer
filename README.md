# Factorizer
Program for factoring integers using Pollard's rho algorithm.
This program can factor 30- to 40-digit numbers in a few minutes.

The algorithm can also be run in parallel with multiple threads.

## Install
Requires:
- [GNU MP Bignum Library](https://gmplib.org/)
- C compiler
- an operating system with POSIX threads

Example command to compile the program:

`gcc -O3 pollard_rho.c -lpthread -lgmp -o pollard_rho`

## Use
`./pollard_rho base10_number [num_threads]`

Prints a factor to standard output if found.

## Author, License
Copyright (C) 2020  Alan Tseng

GNU General Public License v3
