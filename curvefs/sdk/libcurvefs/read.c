#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

void confirm(const std::string& thing) {
    std::string s;
    std::cout << "Continue " << thing << " [y/n]: ";
    std::cin >> s;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "filepath not found" << std::endl;
        return 1;
    }

    auto fd = open(argv[1], O_RDWR | O_CREAT, 0666);
    if (fd < 0)  {
        std::cout << "open() failed: "
                    << strerror(errno) << std::endl;
        return -1;
    }

    confirm("read");
    int nread;
    char out[4096];
    for ( ;; ) {
        auto nread = read(fd, out, 4096);
        if (nread < 0) {
            std::cout << "read() failed: "
                      << strerror(errno) << std::endl;
            return -1;
        } else if (nread == 0) {
            std::cout << "EOF" << std::endl;
            return 0;

        } else {
            std::cout << "read " << nread << " bytes" << std::endl;
        }
    }

    confirm("exit");
    return 0;
}
