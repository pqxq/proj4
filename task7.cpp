// ============================================================================
// Компільовано під: GCC 13.2 / Clang 17.0 (флаг стандарту: -std=c++23)
// Програмний проєкт 4. Варіант 7. Величко Максим, К-24.
// Дослідження багатопотокових структур даних з дрібнозернистим блокуванням.
// ============================================================================

#include "task7.h"

using namespace std;

int ConcurrentDataStore::read(int field_idx) const
{
    if (field_idx < 0 || field_idx >= 3)
    {
        throw out_of_range("Некоректний індекс поля.");
    }
    shared_lock<shared_mutex> lock(mutexes_[field_idx]);
    return data_[field_idx];
}

void ConcurrentDataStore::write(int field_idx, int value)
{
    if (field_idx < 0 || field_idx >= 3)
    {
        throw out_of_range("Некоректний індекс поля.");
    }
    unique_lock<shared_mutex> lock(mutexes_[field_idx]);
    data_[field_idx] = value;
}

ConcurrentDataStore::operator string() const
{
    // Одночасне впорядковане блокування всіх м'ютексів для уникнення взаємоблокувань (deadlocks)
    shared_lock<shared_mutex> lock0(mutexes_[0], defer_lock);
    shared_lock<shared_mutex> lock1(mutexes_[1], defer_lock);
    shared_lock<shared_mutex> lock2(mutexes_[2], defer_lock);
    std::lock(lock0, lock1, lock2);

    return format("[F0: {}, F1: {}, F2: {}]", data_[0], data_[1], data_[2]);
}

static void generate_file(const string &filename, size_t total_ops, const vector<double> &weights)
{
    ofstream out(filename);
    if (!out.is_open())
    {
        throw runtime_error(format("Не вдалося створити файл {}", filename));
    }

    mt19937_64 rng(123);
    discrete_distribution<int> dist(weights.begin(), weights.end());

    for (size_t i = 0; i < total_ops; ++i)
    {
        int op = dist(rng);
        switch (op)
        {
        case 0:
            out << "read 0\n";
            break;
        case 1:
            out << "write 0 1\n";
            break;
        case 2:
            out << "read 1\n";
            break;
        case 3:
            out << "write 1 1\n";
            break;
        case 4:
            out << "read 2\n";
            break;
        case 5:
            out << "write 2 1\n";
            break;
        case 6:
            out << "string\n";
            break;
        }
    }
}

void generate_test_files()
{
    constexpr size_t TotalOps = 300'000;

    // а) Частоти Варіанта 7: r0:5%, w0:5%, r1:30%, w1:5%, r2:24%, w2:1%, str:30%
    generate_file("test_match.txt", TotalOps, {5.0, 5.0, 30.0, 5.0, 24.0, 1.0, 30.0});

    // б) Рівні частоти (~14.28% кожна)
    generate_file("test_equal.txt", TotalOps, {14.28, 14.28, 14.28, 14.28, 14.28, 14.28, 14.32});

    // в) Невідповідні частоти (переважає запис: w0:35%, w1:35%, w2:20%, r0:3%, r1:3%, r2:2%, str:2%)
    generate_file("test_heavy_write.txt", TotalOps, {3.0, 35.0, 3.0, 35.0, 2.0, 20.0, 2.0});
}

vector<Operation> load_operations(const string &filename)
{
    ifstream in(filename);
    if (!in.is_open())
    {
        throw runtime_error(format("Не вдалося відкрити файл {}", filename));
    }

    vector<Operation> ops;
    ops.reserve(300'000);

    string cmd;
    while (in >> cmd)
    {
        if (cmd == "read")
        {
            int idx = 0;
            in >> idx;
            ops.push_back({OpType::Read, idx, 0});
        }
        else if (cmd == "write")
        {
            int idx = 0, val = 0;
            in >> idx >> val;
            ops.push_back({OpType::Write, idx, val});
        }
        else if (cmd == "string")
        {
            ops.push_back({OpType::ToString, 0, 0});
        }
    }
    return ops;
}

double execute_benchmark(const vector<Operation> &ops, size_t num_threads)
{
    ConcurrentDataStore store;
    const size_t total_ops = ops.size();
    const size_t chunk_size = total_ops / num_threads;

    vector<jthread> threads;
    threads.reserve(num_threads);

    auto start_time = chrono::high_resolution_clock::now();

    for (size_t t = 0; t < num_threads; ++t)
    {
        size_t start_idx = t * chunk_size;
        size_t end_idx = (t == num_threads - 1) ? total_ops : start_idx + chunk_size;

        threads.emplace_back([&store, &ops, start_idx, end_idx](){
            for (size_t i = start_idx; i < end_idx; ++i)
            {
                const auto &op = ops[i];
                switch (op.type)
                {
                case OpType::Read:
                    (void)store.read(op.field_idx);
                    break;
                case OpType::Write:
                    store.write(op.field_idx, op.value);
                    break;
                case OpType::ToString:
                    (void)static_cast<string>(store);
                    break;
                }
            } });
    }

    for (auto &th : threads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(end_time - start_time).count();
}

void run_task7()
{
    osyncstream(cout) << string(75, '=') << '\n';
    osyncstream(cout) << "  Програмний проєкт 4: БАГАТОПОТОКОВІ СТРУКТУРИ ДАНИХ (ВАРАНТ 7)\n";
    osyncstream(cout) << string(75, '=') << "\n\n";

    osyncstream(cout) << "Генерація тестових файлів наборів операцій...\n";
    generate_test_files();
    osyncstream(cout) << "Файли успішно згенеровано.\n\n";

    vector<pair<string, string>> test_files = {
        {"а) Відповідають умові (Match)", "test_match.txt"},
        {"б) Рівні частоти (Equal)", "test_equal.txt"},
        {"в) Перевага запису (HeavyWrite)", "test_heavy_write.txt"}};

    osyncstream(cout) << format("{:<35} | {:>10} | {:>10} | {:>10}\n", "Сценарій тестового файлу", "1 потік", "2 потоки", "3 потоки");
    osyncstream(cout) << string(73, '-') << '\n';

    for (const auto &[label, filename] : test_files)
    {
        auto ops = load_operations(filename);

        double t1 = execute_benchmark(ops, 1);
        double t2 = execute_benchmark(ops, 2);
        double t3 = execute_benchmark(ops, 3);

        osyncstream(cout) << format("{:<35} | {:>7.2f} мс | {:>7.2f} мс | {:>7.2f} мс\n", label, t1, t2, t3);
    }
    osyncstream(cout) << string(73, '-') << "\n\n";
}