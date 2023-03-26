#include "hello.h"

#include <string>

int main() {
    Hello hello;

    try {
        hello.run();
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}