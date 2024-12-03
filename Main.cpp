#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <random>

using namespace std;

struct SessionResult {
    int semester;
    string subject;
    int grade;
};

struct Student {
    string name;
    string group;
    vector<SessionResult> sessions;
};

bool isScholarship(const Student& student, const string& group, int semester) {
    if (student.group != group) return false;

    for (const auto& session : student.sessions) {
        if (session.semester == semester && session.grade < 4) {
            return false;
        }
    }

    return true;
}
//без многопоточности
vector<Student> processSequential(const vector<Student>& students, const string& group, int semester) {
    vector<Student> result;
    for (const auto& student : students) {
        if (isScholarship(student, group, semester)) {
            result.push_back(student);
        }
    }
    return result;
}

//с многопоточностью
vector<Student> processParallel(const vector<Student>& students, const string& group, int semester, int threadCount) {
    vector<future<vector<Student>>> futures;

    size_t chunkSize = students.size() / threadCount;
    size_t remainder = students.size() % threadCount;

    // Лямбда-функция, которая обрабатывает часть данных
    auto processChunk = [&](size_t start, size_t end) {
        vector<Student> partialResult;
        for (size_t i = start; i < end; ++i) {
            if (isScholarship(students[i], group, semester)) {
                partialResult.push_back(students[i]);
            }
        }
        return partialResult;
    };

    size_t startIndex = 0;
    //разбиваем студентов на равные части для каждого потока
    for (int i = 0; i < threadCount; ++i) {
        size_t endIndex = startIndex + chunkSize + (i < remainder ? 1 : 0);  // Если есть остаток, добавляем 1 к части

        //запускаем лямбда-функцию в потоке
        futures.push_back(async(launch::async, processChunk, startIndex, endIndex));
        startIndex = endIndex;
    }

    vector<Student> result;
    for (auto& fut : futures) {
        auto partialResult = fut.get();//gолучаем результат из каждого потока
        result.insert(result.end(), partialResult.begin(), partialResult.end());
    }

    return result;
}

vector<Student> generateRandomStudents(int n) {
    vector<Student> students(n);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> gradeGen(2, 5);
    uniform_int_distribution<> semesterGen(1, 2);
    uniform_int_distribution<> groupGen(0, 2);

    vector<string> names = {"Иванов Иван", "Пузина Арина", "Голомедов Максим", "Токмак Яна", "Пупкин Иван", "Сидоров Степан"};
    vector<string> groups = {"A", "B","C"};

    for (int i = 0; i < n; ++i) {
        Student student;
        student.name = names[i % names.size()];
        student.group = groups[groupGen(gen)];

        for (int s = 1; s <= 2; ++s) {
            student.sessions.push_back(SessionResult{s, "Математика", gradeGen(gen)});
            student.sessions.push_back(SessionResult{s, "Физика", gradeGen(gen)});
        }

        students[i] = student;
    }
    return students;
}
void printStudent(const Student& student, int semester) {
    cout << "Имя: " << student.name << ", Группа: " << student.group << "\n";
    cout << "Результаты за семестр " << semester << ":\n";
    for (const auto& session : student.sessions) {
        if (session.semester == semester) {
            cout << "  Предмет: " << session.subject << ", Оценка: " << session.grade << "\n";
        }
    }
}
//малое колво
void test1(const string& group, int semester, int threadCount) {
    vector<Student> smallStudents = {
        {"Иванов Иван", "A", {{1, "Математика", 5}, {1, "Физика", 4}, {2, "История", 3}}},
        {"Петрова Анна", "A", {{1, "Математика", 4}, {1, "Физика", 4}, {2, "История", 5}}},
        {"Сидоров Алексей", "B", {{1, "Математика", 5}, {1, "Физика", 5}, {2, "История", 5}}},
        {"Кузнецова Ольга", "A", {{1, "Математика", 5}, {1, "Физика", 5}, {2, "История", 5}}},
        {"Пупкин Антон", "A", {{1, "Математика", 3}, {1, "Физика", 2}, {2, "История", 4}}}
    };

    // Время обработки без многопоточности
    auto start = chrono::high_resolution_clock::now();
    vector<Student> sequentialResult = processSequential(smallStudents, group, semester);
    auto end = chrono::high_resolution_clock::now();
    cout << "Время без многопоточности: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Количество студентов, получивших стипендию (без многопоточности): " << sequentialResult.size() << endl;
    // Время обработки с многопоточностью
    start = chrono::high_resolution_clock::now();
    vector<Student> parallelResult = processParallel(smallStudents, group, semester, threadCount);
    end = chrono::high_resolution_clock::now();
    cout << "Время с многопоточностью: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
     cout << "Количество студентов, получивших стипендию (с многопоточностью): " << parallelResult.size() << endl;

    // Вывод результатов
    cout << "\nРезультаты (группа " << group << ", семестр " << semester << "):\n";
    for (const auto& student : parallelResult) {
        printStudent(student, semester);
        cout << "----------------------\n";
    }
}
//большое количество студентов
void test2(const string& group, int semester, int threadCount) {
    vector<Student> largeStudents = generateRandomStudents(4000234);

    // Время обработки без многопоточности
    auto start = chrono::high_resolution_clock::now();
    vector<Student> sequentialResult = processSequential(largeStudents, group, semester);
    auto end = chrono::high_resolution_clock::now();
    cout << "Время без многопоточности: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Количество студентов, получивших стипендию (без многопоточности): " << sequentialResult.size() << endl;

    // Время обработки с многопоточностью
    start = chrono::high_resolution_clock::now();
    vector<Student> parallelResult = processParallel(largeStudents, group, semester, threadCount);
    end = chrono::high_resolution_clock::now();
    cout << "Время с многопоточностью: " << chrono::duration<double>(end - start).count() << " секунд" << endl;
    cout << "Количество студентов, получивших стипендию (с многопоточностью): " << parallelResult.size() << endl;
}

int main() {
    setlocale(LC_ALL,"Russian");
    string group = "A";
    int semester = 1;
    int threadCount = 4;

    test1(group, semester, threadCount);
    test2(group, semester, threadCount);

    return 0;
}