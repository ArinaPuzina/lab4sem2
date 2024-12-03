#include <iostream>
#include <thread>
#include <semaphore>
#include <vector>
#include <chrono>
#include <mutex>

#include "utils/utils.hpp"

#define N 5
#define numIter 100

using namespace std;

// Создаем семафор, ограничивающий доступ к критической секции до 2 потоков
counting_semaphore<1> semaphore(1);

// Мьютекс для синхронизации доступа к cout
mutex cout_mutex;

void run() {
    auto start = chrono::steady_clock::now();

    semaphore.acquire(); 

    string output; // буфер вывода
    for (int i = 0; i < numIter; i++) {
        output += generateRandom();
        output += " ";
    }

    // Синхронизированный вывод
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << output << endl;
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    semaphore.release(); 

    // Синхронизированный вывод времени выполнения
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    }
}

template <typename T>
void primitive(T func) {
    vector<thread> threads;
    for (int i = 0; i < N; i++) {
        threads.emplace_back(func);
    }

    for (auto& th : threads) {
        th.join();
    }
}

int main() {
    primitive(run);
    return 0;
}