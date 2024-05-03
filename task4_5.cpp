#include <iostream>
#include <deque>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#include <windows.h>

sem_t* sem = nullptr;
pid_t pid = 0;
constexpr uint32_t SLEEP_MS = 10;

void lock_mtx_or_sem(std::mutex& mtx) {
    if(sem == nullptr)
        mtx.lock();
    else
        sem_wait(sem);
}

void unlock_mtx_or_sem(std::mutex& mtx) {
    if(sem == nullptr)
        mtx.unlock();
    else
        sem_post(sem);
}

void task_4_writer(std::mutex& mtx, std::deque<std::string>& deq, const int writer_num, const int write_count) {
    for (int i = 0; i < write_count; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        lock_mtx_or_sem(mtx);
        deq.push_back("news #" + std::to_string(i) + " from writer #" + std::to_string(writer_num));
        unlock_mtx_or_sem(mtx);
    }
}

void task_4_reader(std::mutex& mtx, std::deque<std::string>& deq, const int reader_num, int& news_left) {
    pid_t currentPid = getpid();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        lock_mtx_or_sem(mtx);
        if (!news_left) {
            unlock_mtx_or_sem(mtx);
            break;
        }
        std::string n = deq[deq.size() - 1];
        deq.pop_back();
        news_left--;
        unlock_mtx_or_sem(mtx);
        printf("[%d] reader #%d read news: \"%s\"\n", currentPid, reader_num, n.c_str());
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

void task_5_thread(std::mutex* forks, const int p_num, const int loop_count, const int philosopher_count) {
    pid_t currentPid = getpid();
    for (int i = 0; i < loop_count; i++) {
        int state = -1;
        while (state < PUT_FORK2) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            state++;
            if(state == TAKE_FORK1) {
                forks[p_num].lock();
                printf("[%d] philosopher #%d: %s\n", currentPid, p_num, task_5_str_state(state).c_str());
                state++;
                forks[(p_num+1) % philosopher_count].lock();
            }

            printf("[%d] philosopher #%d: %s\n", currentPid, p_num, task_5_str_state(state).c_str());
            if(state == PUT_FORK1) {
                forks[p_num].unlock();
                state++;
                printf("[%d] philosopher #%d: %s\n", currentPid, p_num, task_5_str_state(state).c_str());
                forks[(p_num+1) % philosopher_count].unlock();
            }
        }
    }
}

void task5() {
    /*
     * В тій же програмі реалізувати потоки для філософів, що обідають,
     * Потік повинен забезпечити повний цикл операцій (думає, бере одну виделку, бере другу виделку, обідає, кладе одну виделку, кладе другу виделку), які виконуються задану кількість разів.
     * Перевірити відсутність гонок та блокувань при використанні потоків в одній програмі.
     */

    //  1 2 3 4
    // f f f f

    std::vector<std::thread> threads;
    constexpr int philosopher_count = 4;
    constexpr int loop_count = 2;
    auto* forks = new std::mutex[philosopher_count];

    for (int i = 0; i < philosopher_count; i++) {
        threads.emplace_back(task_5_thread, forks, i, loop_count, philosopher_count);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    delete[] forks;
}

void CALLBACK timer_cb(void* arg, const uint32_t timerLowValue, const uint32_t timerHighValue) {
    if(getpid() != pid)
        return;

    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(timerLowValue);
    UNREFERENCED_PARAMETER(timerHighValue);

    if(fork() == 0) {
        task3_4();
        task5();
        exit(0);
    }
}

void task6() {
    void* timer = CreateWaitableTimer(nullptr, false, "os-lb8-task6");
    constexpr uint64_t dueInt = -1 * 10000000;
    constexpr LARGE_INTEGER due{
        dueInt & 0xFFFFFFFF,
        static_cast<int32_t>(dueInt >> 32),
    };

    if (!SetWaitableTimer(timer, &due, 100, timer_cb, nullptr, false)) {
        printf("Failed to set timer\n");
        return;
    }

    for (int i = 0; i < 10; i += 1) {
        SleepEx(INFINITE, TRUE);
    }
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        printf("Using semaphore %s to synchronize threads\n", argv[1]);
        sem = sem_open(argv[1], 0, 0644, 1);
        pid = getpid();

        task6();

        sem_destroy(sem);
        return 0;
    }

    task3_4();
    task5();

    return 0;
}
