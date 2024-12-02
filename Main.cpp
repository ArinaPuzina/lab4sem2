#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <random>
#include <thread>

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

//получил ли студент стипендию
bool isScholarship(const Student& student, const string& group, int semester) {
    if (student.group != group) return false;
    int totalGrades = 0, count = 0;
    for (const auto& session : student.sessions) {
        if (session.semester == semester) {
            totalGrades += session.grade;
            count++;
        }
    }
    return count > 0 && (totalGrades / static_cast<double>(count)) >= 4.0;
}

// Функция обработки без многопоточности
vector<Student> processSequential(const vector<Student>& students, const string& group, int semester) {
    vector<Student> result;
    for (const auto& student : students) {
        if (isScholarship(student, group, semester)) {
            result.push_back(student);
        }
    }
    return result;
}

// Функция обработки с многопоточностью
vector<Student> processParallel(const vector<Student>& students, const string& group, int semester, int threadCount) {
    vector<future<vector<Student>>> futures;
    size_t chunkSize = students.size() / threadCount;
    size_t remainder = students.size() % threadCount;

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
    for (int i = 0; i < threadCount; ++i) {
        size_t endIndex = startIndex + chunkSize + (i < remainder ? 1 : 0);
        futures.push_back(async(launch::async, processChunk, startIndex, endIndex));
        startIndex = endIndex;
    }

    // Предварительное резервирование места в векторе для результатов
    vector<Student> result;
    result.reserve(students.size());  // Резервируем место для всех студентов

    // Собираем результаты из потоков
    for (auto& fut : futures) {
        auto partialResult = fut.get();
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
    uniform_int_distribution<> groupGen(0, 1);

    vector<string> names = {"Ivanov Ivan", "Petrova Anna", "Sidorov Alex", "Smirnov Ivan", "Kuznetsov Alex"};
    vector<string> groups = {"A1", "B2"};

    for (int i = 0; i < n; ++i) {
        Student student;
        student.name = names[i % names.size()];
        student.group = groups[groupGen(gen)];

        for (int s = 1; s <= 2; ++s) { // 2 семестра
            student.sessions.push_back(SessionResult{s, "Math", gradeGen(gen)});
            student.sessions.push_back(SessionResult{s, "Physics", gradeGen(gen)});
        }

        students[i] = student;
    }

    return students;
}

int main() {
    setlocale(LC_ALL,"Russian");
    vector<Student> smallStudents = generateRandomStudents(56780); // Сгенерируем студентов для теста
    string group = "A1";
    int semester = 1;
    int threadCount = 4;
    //thread::hardware_concurrency();// Динамически определяем количество потоков

    // Время обработки без многопоточности
    auto start = chrono::high_resolution_clock::now();
    vector<Student> sequentialResult = processSequential(smallStudents, group, semester);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> durationSequential = end - start;
    cout << "Время обработки без многопоточности: " << durationSequential.count() << " секунд" << endl;
    // Время обработки с многопоточностью
    start = chrono::high_resolution_clock::now();
    vector<Student> parallelResult = processParallel(smallStudents, group, semester, threadCount);
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> durationParallel = end - start;
    cout<<"Thread count: "<<threadCount<<endl;
    cout << "Время обработки с многопоточностью: " << durationParallel.count() << " секунд" << endl;

    return 0;
}