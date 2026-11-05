// ============================================================================
// Компільовано під: GCC 13.2 / Clang 17.0 (флаг стандарту: -std=c++23)
// Програмний проєкт 4. Варіант 7. Величко Максим, К-24.
// Дослідження багатопотокових структур даних з дрібнозернистим блокуванням.
// ============================================================================

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <shared_mutex>
#include <mutex>
#include <format>
#include <syncstream>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <exception>

// Типи операцій зі структурою даних
enum class OpType
{
    Read,
    Write,
    ToString
};

// Структура запису дій для виконання
struct Operation
{
    OpType type;
    int field_idx = 0;
    int value = 0;
};

// Багатопотокова структура даних для Варіанта 7 (m = 3 полів)
class ConcurrentDataStore
{
private:
    int data_[3]{0, 0, 0};
    mutable std::shared_mutex mutexes_[3];

public:
    ConcurrentDataStore() = default;

    // Читання окремого поля (дозволяє паралельне читання)
    int read(int field_idx) const;

    // Запис окремого поля (ексклюзивне блокування тільки даного поля)
    void write(int field_idx, int value);

    // Приведення структури до рядка (атомарний знімок усіх полів)
    explicit operator std::string() const;
};

// Функції генерації тестових файлів та проведення бенчмарків
void generate_test_files();
std::vector<Operation> load_operations(const std::string &filename);
double execute_benchmark(const std::vector<Operation> &ops, size_t num_threads);
void run_task7();