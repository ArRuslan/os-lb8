#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <valarray>
#include <vector>

#include <windows.h>

struct Task1ThreadParam {
    uint32_t threadNum;
    uint8_t* counter;
    std::mutex* mtx;
};

uint32_t WINAPI task1_thread(void* param) {
    const auto par = static_cast<Task1ThreadParam*>(param);
#ifndef NDEBUG
    const uint32_t num = par->threadNum;
    printf("Thread #%d started: %ld\n", num, clock());
#endif
    par->mtx->lock();
    (*par->counter)++;
    par->mtx->unlock();

    free(param);

    for(int i = 0; i < 4 * 1024; i++) {
        const clock_t wait_for = clock() + 4 * 1024;
        while(clock() < wait_for) {}
    }

#ifndef NDEBUG
    printf("Thread #%d finished: %ld\n", num, clock());
#endif
    return 0;
}

void task1() {
    /*
     * Запустити 10 потоків, в яких спочатку видати інформацію про початок потокової функції, в  кінці – про її завершення.
     * В потоковій функції використовувати цикл, завдяки якому час виконання потокової функції буде більше одного кванта часу.
     * Рахувати, скільки разів буде викликатися потокова функція.
     * Чекати завершення хоча б одного потоку
     * Рахувати, скільки разів буде викликатися потокова функція.
     * Зробити так, щоб інформація  про початок та кінець для потокової функції виводився в файл тільки в режимі DEBUG, забезпечивши можливість використання функції різними програмами.
     */

    uint8_t counter;
    std::mutex mtx;
    uint32_t threadId;
    const auto threads = new void*[10];
    for(uint32_t i = 0; i < 10; i++) {
        const auto param = static_cast<Task1ThreadParam*>(malloc(sizeof(Task1ThreadParam)));
        param->threadNum = i;
        param->mtx = &mtx;
        param->counter = &counter;
        threads[i] = CreateThread(nullptr, 0, &task1_thread, param, 0, &threadId);
    }

    WaitForMultipleObjects(10, threads, true, INFINITE);
    delete[] threads;

    printf("%d threads finished\n", counter);
}

uint32_t WINAPI task2_thread(void* param) {
    constexpr uint32_t n = 1024 * 1024 * 64;
    uint32_t a = 0, b = 1, c = 0;

    for (int i = 1; i <= n; ++i) {
        if(i == 1 || i == 2)
            continue;
        c = a + b;
        a = b;
        b = c;
    }

    return c;
}

void task2() {
    /*
     * Визначити максимальну кількість потоків, які можна запустити та чекати їх завершення.
     * Cтворити задану кількість потоків, які формують кількість викликів потокової функції; чекати їх завершення.
     * Якщо кількість викликів потокової функції не співпадає з кількістю потоків, вийти з циклу.
     */

    const uint32_t shouldReturn = task2_thread(nullptr);
    const uint32_t threadCount = std::thread::hardware_concurrency();
    const auto threads = new void*[threadCount];
    uint32_t threadId;
    for(int i = 0; i < threadCount; i++) {
        threads[i] = CreateThread(nullptr, 0, &task2_thread, nullptr, 0, &threadId);
    }

    WaitForMultipleObjects(threadCount, threads, true, INFINITE);

    for(int i = 0; i < threadCount; i++) {
        uint32_t code;
        GetExitCodeThread(threads[i], &code);
        if(code == shouldReturn)
            printf("Thread %d finished successfully\n", i);
        else
            printf("Thread %d finished with invalid return code\n", i);
        CloseHandle(threads[i]);
    }

    delete[] threads;
}

void task6() {
    /*
     * Створити додаткову програму, яка запускає декілька раз першу програму з потоками відповідно з заданим розкладом (WaitableTimer).
     * Внести зміну в об’єкти синхронізації, які потрібні.
     */

    // TODO: what
}

int main() {
    //task1();
    task2();
    /*task3_4();
    task5();
    task6();*/

    return 0;
}
