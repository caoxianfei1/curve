



int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "filepath not found" << std::endl;
        return 1;
    }

    auto rc = open(argv[1], O_WRONLY|O_CREAT, 0666);
    if (rc < 0)  {
        std::cout << "open() failed: "
                    << strerror(errno) << std::endl;
        return -1;
    }

    confirm("exit");

    return 0;
}

