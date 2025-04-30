#include <termios.h>
#include <unistd.h>

#include "check.hpp"
#include <fcntl.h>
#include <poll.h>

int acquire_new_terminal() {

  int master_fd = check(posix_openpt(O_RDWR|O_NOCTTY));


  check(grantpt(master_fd));
  struct termios attr;

  check(tcgetattr(master_fd,&attr));
  attr.c_lflag |= ISIG;       // Enable control symbols processing
  check(tcsetattr(master_fd,TCSANOW,&attr));
  check(unlockpt(master_fd));

  return master_fd;

}

int main(){
  printf("PID: %d\n", getpid());
  int master_fd = acquire_new_terminal();
  const char* slave_name = check(ptsname(master_fd));
  close(open(slave_name, O_RDWR|O_NOCTTY)); // close() sets HANG-UP state, needed for poll() to work

  printf("The slave-end of the terminal is %s\n", slave_name);
  printf("Use cat %s to read\n",slave_name);

  while (1) {
    pollfd pfd{
      .fd = master_fd,
      .events = POLLOUT|POLLHUP,
    };

    check(poll(&pfd, 1, -1));

    if (pfd.revents & POLLHUP) {
      sleep(1);  // POLLHUP = slave-end in not opened
    }
    else {
      const char msg[] = "BEGONE\n";
      check(write(master_fd, msg, sizeof(msg)-1 ));
      const char CONTROLS[] = {0x04}; // EOT=EOF on the slave-end (read() in other process will return 0)
      check(write(master_fd, CONTROLS, sizeof(CONTROLS) ));
    }
  }


}