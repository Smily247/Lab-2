#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

static const long Num_To_Add = 1000000000;
static const double Scale = 10.0 / RAND_MAX;

long add_serial(const char *numbers) {
    long sum = 0;
    for (long i = 0; i < Num_To_Add; i++) {
        sum += numbers[i];
    }
    return sum;
}

long add_parallel(const char *numbers) {
    long Global_sum = 0;
    long local_Sum = 0;
    long num_of_threads = omp_get_num_threads();
    long my_first_i = 0;
    long my_last_i = 0;
    long size_of_thread = (long) round((Num_To_Add / num_of_threads) + .5);
    my_first_i = omp_get_thread_num() * size_of_thread;// thread id * size per thread
    my_last_i = my_first_i + size_of_thread;// my first + size per thread


#pragma omp parallel for reduction(+:local_Sum)

    for (long i = my_first_i; i < my_last_i; (i++)) {
        local_Sum += numbers[i];
    }
# pragma omp critical
    Global_sum = +local_Sum;
    return Global_sum;
}

int main() {
    char *numbers = malloc(sizeof(long) * Num_To_Add);

    long chunk_size = Num_To_Add / omp_get_max_threads();
#pragma omp parallel num_threads(omp_get_max_threads())
    {
        int p = omp_get_thread_num();
        unsigned int seed = (unsigned int) time(NULL) + (unsigned int) p;
        long chunk_start = p * chunk_size;
        long chunk_end = chunk_start + chunk_size;
        for (long i = chunk_start; i < chunk_end; i++) {
            numbers[i] = (char) (rand_r(&seed) * Scale);
        }
    }

    struct timeval start, end;

    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);
    long sum_s = add_serial(numbers);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("Timing parallel...\n");
    gettimeofday(&start, NULL);
    long sum_p = add_parallel(numbers);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    printf("Sum serial: %ld\nSum parallel: %ld", sum_s, sum_p);

    free(numbers);
    return 0;
}

