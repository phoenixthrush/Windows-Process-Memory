#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep(ms) usleep((ms) * 1000)
#endif

int main() {
    char message[] = "Hello, World!";
    while (1) {
        printf("%s\n", message);
        fflush(stdout);
        sleep(1000);
    }
    return 0;
}
