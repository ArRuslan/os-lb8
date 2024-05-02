#include <iostream>
#include <semaphore.h>
#include <fcntl.h>

sem_t* sem;

int main() {
    sem = sem_open("/os-lb8-task345", O_CREAT, 0644, 1);

    std::system("./os_task45.exe /os-lb8-task345");

    sem_destroy(sem);
    return 0;
}
