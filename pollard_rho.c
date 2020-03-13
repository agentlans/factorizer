/*  Factorizer : program for factoring integers
    Copyright (C) 2020  Alan Tseng

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gmp.h>
#include <pthread.h>

// Example compilation line:
// gcc -O3 pollard_rho.c -lpthread -lgmp -o pollard_rho
// Usage: ./pollard_rho base10_number [num_threads]

// g(x) = x^2 + 1 (mod n)
void g(mpz_t x, mpz_t n)
{
	mpz_mul(x, x, x);
	mpz_add_ui(x, x, 1);
	mpz_mod(x, x, n);
}

// Uses Pollard's rho algorithm to factor n and saves factor to result.
void pollard_rho(mpz_t result, mpz_t n, mpz_t start)
{
	mpz_t x, y, d;
	mpz_init_set(x, start);
	mpz_init_set(y, start);
	mpz_init_set_ui(d, 1);

	long counter = 0;
	bool overwrite_result = true;
	while (mpz_cmp_ui(d, 1) == 0) {
		if (counter % 1000000 == 0) {
			//printf("Checking...\n");
			// There's already a solution so don't overwrite it
			if (mpz_cmp_ui(result, 0) != 0) {
				overwrite_result = false;
				break;
			}
		}

		// Run y twice for each run of x
		g(x, n);
		g(y, n); g(y, n);

		mpz_t diff;
		mpz_init(diff);

		// See if we have a factor
		mpz_sub(diff, x, y);
		mpz_abs(diff, diff);
		mpz_gcd(d, diff, n);
		mpz_clear(diff);

		counter++;
	}

	if (overwrite_result) {
		mpz_set(result, d);
		// printf("Found a factor!\n");
	}
	mpz_clear(x);
	mpz_clear(y);
	mpz_clear(d);
}

// Holds the start numbers for different threads
mpz_t *starts;
pthread_t *threads;

struct thread_input
{
	mpz_t *result;
	mpz_t *n;
	mpz_t *start;
};

// Wraps the factorization routine into a thread
void *wrapper(void *in)
{
	struct thread_input *in2 = (struct thread_input *) in;
	pollard_rho(*(in2->result), *(in2->n), *(in2->start));
	return NULL;
}

int main(int argc, char *argv[])
{
	int num_threads = 1;
	char *n_str = "";
	if (argc == 2) {
		// Read the number
		n_str = argv[1];
	} else if (argc == 3) {
		// Read the number of threads
		n_str = argv[1];
		num_threads = atoi(argv[2]);
		if (num_threads < 1) {
			printf("Error. Must have at least one thread.\n");
			return 1;
		}
	} else {
		printf("Usage: ./pollard_rho number_to_factor [number_of_threads]\n");
		return 1;
	}

	mpz_t n; // Number to be factored
	int n_err = mpz_init_set_str(n, n_str, 10);
	if (n_err != 0) {
		printf("Error. Not a valid base 10 number.\n");
		mpz_clear(n);
		return 1;
	}

	gmp_randstate_t rnd;
	gmp_randinit_default(rnd);

	// Variable that will hold a factor
	mpz_t factor;
	mpz_init(factor);

	starts = calloc(num_threads, sizeof(mpz_t));
	threads = calloc(num_threads, sizeof(pthread_t));
	if (!starts || !threads) {
		printf("Couldn't initialize the threads.");
		return 1;
	}

	for (int i = 0; i < num_threads; ++i) {
		// Random number for a thread
		mpz_init(starts[i]);
		mpz_urandomm(starts[i], rnd, n);
		// gmp_printf("Start: %Zd\n", starts[i]);

		// Set other parameters for the thread
		struct thread_input in;
		in.result = &factor;
		in.n = &n;
		in.start = starts + i;

		// Launch thread
		pthread_create(&threads[i], NULL, wrapper, (void *) &in);
	}

	// Wait until all threads are finished
	for (int i = 0; i < num_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	gmp_printf("%Zd\n", factor);
	// gmp_printf("%Zd is a factor of %Zd\n", factor, n);

	// Clean up
	for (int i = 0; i < num_threads; ++i) {
		mpz_clear(starts[i]);
	}
	free(starts);
	free(threads);

	mpz_clear(factor);
	gmp_randclear(rnd);
	mpz_clear(n);

	pthread_exit(NULL);

	return 0;
}


