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

void task_4_writer(std::mutex& mtx, std::deque<std::string>& deq, const int writer_num, const int write_count) {
    for (int i = 0; i < write_count; i++) {
        mtx.lock();
        deq.push_back("news #" + std::to_string(i) + " from writer #" + std::to_string(writer_num));
        mtx.unlock();
    }
}

void task_4_reader(std::mutex& mtx, std::deque<std::string>& deq, const int reader_num, int& news_left) {
    while (true) {
        mtx.lock();
        if (!news_left) {
            mtx.unlock();
            break;
        }
        std::string n = deq[deq.size() - 1];
        deq.pop_back();
        news_left--;
        mtx.unlock();
        printf("reader #%d read news: \"%s\"\n", reader_num, n.c_str());
    }
}

void task3_4() {
    /*
     * Реалізувати потоки для читача та письменника.
     * Реалізувати головну програму, в якій створено декілька потоків обох типів.
     * Потік письменника записує запис в кінець списку новин.
     * Потік читача читає останню новину.
     */

    std::mutex mtx;
    std::deque<std::string> deq;
    std::vector<std::thread> threads;
    int total_news = 64;
    constexpr int writer_count = 8;
    constexpr int reader_count = 4;

    for (int i = 0; i < writer_count; i++) {
        threads.emplace_back(task_4_writer, std::ref(mtx), std::ref(deq), i, total_news / writer_count);
    }

    for (int i = 0; i < reader_count; i++) {
        threads.emplace_back(task_4_reader, std::ref(mtx), std::ref(deq), i, std::ref(total_news));
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

enum Task5State {
    THINKING = 0,
    TAKE_FORK1 = 1,
    TAKE_FORK2 = 2,
    EATING = 3,
    PUT_FORK1 = 4,
    PUT_FORK2 = 5,
};

std::string task_5_str_state(const int state) {
    switch (state) {
    case THINKING:
        return "thinking";
    case TAKE_FORK1:
        return "took fork 1";
    case TAKE_FORK2:
        return "took fork 2";
    case EATING:
        return "eating";
    case PUT_FORK1:
        return "put fork 1 down";
    case PUT_FORK2:
        return "put fork 2 down";
    default:
        return "unreachable";
    }
}

void task_5_thread(std::mutex& mtx, const int p_num, const int loop_count) {
    for (int i = 0; i < loop_count; i++) {
        int state = -1;
        while(state < PUT_FORK2) {
            mtx.lock();
            state++;
            printf("philosopher #%d: %s\n", p_num, task_5_str_state(state).c_str());
            mtx.unlock();
        }
    }
}

void task5() {
    /*
     * В тій же програмі реалізувати потоки для філософів, що обідають,
     * Потік повинен забезпечити повний цикл операцій (думає, бере одну виделку, бере другу виделку, обідає, кладе одну виделку, кладе другу виделку), які виконуються задану кількість разів.
     * Перевірити відсутність гонок та блокувань при використанні потоків в одній програмі.
     */

    std::mutex mtx;
    std::vector<std::thread> threads;
    constexpr int philosopher_count = 4;
    constexpr int loop_count = 2;

    for (int i = 0; i < philosopher_count; i++) {
        threads.emplace_back(task_5_thread, std::ref(mtx), i, loop_count);
    }

    for (auto& thread : threads) {
        thread.join();
    }
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
