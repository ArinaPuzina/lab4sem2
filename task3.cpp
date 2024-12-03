#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#define N 5
#define THINKING 2
#define HUNGRY 1
#define EATING 0
#define LEFT (phnum + 4) % N
#define RIGHT (phnum + 1) % N

using namespace std;

int state[N];  // Состояния философов
int phil[N] = { 0, 1, 2, 3, 4 };

mutex mtx; 
condition_variable S[N];  // Переменные условия для каждого философа

void test(int phnum) {
    if (state[phnum] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[phnum] = EATING;
        cout << "Philosopher " << phnum + 1 << " is now eating" << endl;
        S[phnum].notify_one();  // Уведомление философа, что он может начать есть
    }
}


void take_fork(int phnum) {
    unique_lock<mutex> lock(mtx);  

    // Философ голоден
    state[phnum] = HUNGRY;
    cout << "Philosopher " << phnum + 1 << " is Hungry" << std::endl;

    // Попытка начать есть, если соседи не едят
    test(phnum);

    // Если философ не может есть, он будет ждать уведомления
    while (state[phnum] != EATING) {
        S[phnum].wait(lock);  // Ожидание, пока философ не получит уведомление
    }

    this_thread::sleep_for(chrono::milliseconds(1000));
}

// Функция, чтобы философ положил вилки
void put_fork(int phnum) {
    unique_lock<mutex> lock(mtx);  // Используем переименованный мьютекс `mtx`

    // Философ начинает думать
    state[phnum] = THINKING;
    cout << "Philosopher " << phnum + 1 << " is thinking" << std::endl;

    // Проверяем соседей, могут ли они начать есть
    test(LEFT);
    test(RIGHT);
}

// Функция, которую выполняет каждый философ
void philosopher(int num) {
    while (true) {
        take_fork(num);   // Философ пытается взять вилки
        put_fork(num);    // Философ кладет вилки и начинает думать
    }
}

int main() {
    thread threads[N];

    for (int i = 0; i < N; i++) {
        threads[i] = thread(philosopher, i);
        cout << "Philosopher " << i + 1 << " is thinking" << std::endl;
    }

    for (int i = 0; i < N; i++) {
        threads[i].join();
    }

    return 0;
}