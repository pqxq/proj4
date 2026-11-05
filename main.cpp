// ============================================================================
// Компільовано під: GCC 13.2 / Clang 17.0 (флаг стандарту: -std=c++23)
// Програмний проєкт 4. Варіант 7. Величко Максим, К-24.
// Дослідження багатопотокових структур даних з дрібнозернистим блокуванням.
// ============================================================================

#include "task7.h"

#include <iostream>
#include <format>
#include <exception>
#include <cstdlib>

int main()
{
    try
    {
        run_task7();
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << std::format("Критична помилка: {}\n", e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Невідома фатальна помилка\n";
        return EXIT_FAILURE;
    }
}