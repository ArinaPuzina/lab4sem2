#include <iostream>
#include <vector>
#include <thread>
#include <semaphore>
#include <chrono>
#include <mutex>

#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

mutex mtx;

void runMutex() {
    auto start = chrono::steady_clock::now();
    mtx.lock();
    for(int i = 0; i < numIter; i++) {
        cout << generateRandom() << " ";
        //this_thread::sleep_for(chrono::milliseconds(10));
    }
    cout << endl;
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    mtx.unlock();
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
    primitive(runMutex);

    return 0;
}