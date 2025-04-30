#include "check.hpp"
#include <termios.h>
#include <unistd.h>


void change_terminal_mode() {
    struct termios attr;

    check(tcgetattr(0,&attr));
    attr.c_lflag &= ~(ICANON|IEXTEN|ISIG);       // Exit canonical mode, enable special processsing, disable signal generation
    attr.c_lflag &= ~(ECHO|ECHONL);              // do not show input
    attr.c_iflag &= ~(BRKINT|ICRNL|ISTRIP|IXON); // disable SIGINT on Ctrl+C, disable replacing '\r' by '\n', disable Start/Stop signals from console
    attr.c_cc[VMIN]=1;                           // return from read() when at least 1 byte is avalable
    attr.c_cc[VTIME]=0;                          // max wait time is infinite

    check(tcsetattr(0,TCSANOW,&attr));
}

int main() {

    change_terminal_mode();

    while (1) {
        volatile char c;
        check(read(0, (char*)&c, 1));
        check(write(1, (char*)&c, 1));

    }

}