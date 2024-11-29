#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <string>
#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

// SpinWait с использованием atomic_bool
atomic_bool ready(false); // Условие для активного ожидания

void spin_wait() {
    // Ожидаем до тех пор, пока ready не станет true
    while (!ready.load(memory_order_acquire)) {
        this_thread::yield(); // Уступаем другим потокам
    }
}

// Мьютекс для синхронизации вывода
mutex cout_mutex;

void run() {
    auto start = chrono::steady_clock::now();
    
    spin_wait(); // Активное ожидание

    // Генерация случайных символов
    string output; // Локальный буфер для вывода
    for (int i = 0; i < numIter; i++) {
        output += generateRandom();
        output += " ";
    }

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << output << endl;
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

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

    this_thread::sleep_for(chrono::milliseconds(100));

    // Устанавливаем ready в true, чтобы потоки прекратили активное ожидание
    ready.store(true, memory_order_release);

    for (auto& th : threads) {
        th.join();
    }
}

int main() {
    primitive(run);
    return 0;
}