/*
Exercise 1: Parallel Array Sum
Write a program that:

Creates 4 threads
Each thread sums 1/4 of a large array
Combine results for final sum
Compare performance with single-threaded version
*/

#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>
#include <cstdlib>

struct Split {
    int* arr;
    int start;
    int end;
};

void* thread_function(void* arg) {
    Split split = *(Split*)arg;
    int* res = new int(0);

    for (int i = split.start; i < split.end; ++i) {
        *res += split.arr[i];
    }

    return (void*)res;
}

int main() {
    int array_size = 25;
    int arr[array_size];

    for (int i = 0; i < array_size; ++i) {
        arr[i] = i;
    }

    pthread_t thread1, thread2, thread3, thread4;

    Split split1 {arr, 0, array_size / 4};
    Split split2 {arr, array_size / 4, array_size / 2};
    Split split3 {arr, array_size / 2, (array_size / 4) * 3};
    Split split4 {arr, (array_size / 4) * 3, array_size};

    pthread_create(&thread1, nullptr, thread_function, &split1);
    pthread_create(&thread2, nullptr, thread_function, &split2);
    pthread_create(&thread3, nullptr, thread_function, &split3);
    pthread_create(&thread4, nullptr, thread_function, &split4);

    void* ret1;
    void* ret2;
    void* ret3;
    void* ret4;

    pthread_join(thread1, &ret1);
    pthread_join(thread2, &ret2);
    pthread_join(thread3, &ret3);
    pthread_join(thread4, &ret4);

    int total_sum = *(int*)ret1 + *(int*)ret2 + *(int*)ret3 + *(int*)ret4;

    std::cout << total_sum << '\n';

    delete (int*)ret1;
    delete (int*)ret2;
    delete (int*)ret3;
    delete (int*)ret4;

    return 0;
}