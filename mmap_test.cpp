#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>



int main() {
    const char* name = "/gateway_ring_buf";
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    struct Order {
    int side;
    int limitPrice;
    int clientId;
};
#define BUFLEN 100
ftruncate(fd, sizeof(Order) * BUFLEN);


    pid_t c_pid = fork();
  
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (c_pid > 0) {
        // Sender process
        data->count = 42;
        data->value = 3.14;

        //  wait(nullptr);
        std::cout << "printed from parent process " << getpid()
             << std::endl;
    }
    else {
        // Receiver process
        std::cout << "Count: " << data->count << std::endl;
        std::cout << "Value: " << data->value << std::endl;

        std::cout << "printed from child process " << getpid()
             << std::endl;
    }


    munmap(data, sizeof(Order) * BUFLEN);
    shm_unlink(name);

    return 0;
}
