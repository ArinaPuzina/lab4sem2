#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

class SpinLock {
    atomic_flag lock_flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (lock_flag.test_and_set(memory_order_acquire)) {}
    }

    void unlock() {
        lock_flag.clear(memory_order_release);
    }
};

SpinLock spinlock;

void run() {
    auto start = chrono::steady_clock::now();
    spinlock.lock();
    for(int i = 0; i < numIter; i++) {
        cout << generateRandom() << " ";
    }
    cout << endl;
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    spinlock.unlock();
    cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
}

template <typename T>
void primitive(T func){
    vector<thread> threads;
    for( int i = 0; i < N; i++) {
        threads.emplace_back(func);
    }

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }
}

int main() {
    primitive(run);
    return 0;
}