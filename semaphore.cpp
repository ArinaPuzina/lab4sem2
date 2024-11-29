#include <iostream>
#include <thread>
#include <semaphore>
#include <vector>
#include <chrono>

#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

counting_semaphore<2> semaphore(2);

void run() {
    auto start = chrono::steady_clock::now();
    semaphore.acquire(); //захватывает семафор
    for(int i = 0; i < numIter; i++) {
        cout << generateRandom() << " ";
        //this_thread::sleep_for(chrono::milliseconds(10));
    }
    cout << endl;
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    semaphore.release();
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