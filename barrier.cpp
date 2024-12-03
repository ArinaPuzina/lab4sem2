#include <iostream>
#include <thread>
#include <barrier>
#include <vector>
#include <chrono>
#include <mutex>

#include "support.hpp"

#define N 5
#define numIter 100

using namespace std;

// Барьер для синхронизации
barrier point(N);

// Мьютекс для синхронизации вывода
mutex mtx;

void run() {
    auto start = chrono::steady_clock::now();

    // Ожидание на барьере
    point.arrive_and_wait();

    vector<char> random_symbols;
    for (int i = 0; i < numIter; i++) {
        random_symbols.push_back(generateRandom());
    }

    //Синхронизированный вывод
    {
        lock_guard<mutex> lock(mtx);
        for (auto num : random_symbols) {
            cout << num << " ";
        }
        cout << endl;
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    // Вывод времени выполнения
    {
        lock_guard<std::mutex> lock(mtx);
        cout << "Elapsed time: " << elapsed.count() << " seconds" << endl;
    }
}

template <typename T>
void primitive(T func) {
    vector<thread> threads;
    for (int i = 0; i < N; i++) {
        threads.emplace_back(func);
    }

    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    primitive(run);
    return 0;
}