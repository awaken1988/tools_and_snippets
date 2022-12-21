#include <syscall.h>
#include <unistd.h>

int main() {    
    int a = 0;

    const char msg[] = "Hello World!";
    syscall(1, 0, msg, sizeof(msg)-1);

    return a+1;
}