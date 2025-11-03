#include <termios.h>

struct termios old, current;

void terminal_init_raw() {
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON; // disable buffered IO
    current.c_lflag &= ~ECHO; // disable echo
    tcsetattr(0, TCSANOW, &current);
}

void terminal_deinit_raw() {
    tcsetattr(0, TCSANOW, &old);
}
