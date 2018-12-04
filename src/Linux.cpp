#ifdef __LINUX__
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

const char* timestamp(){
    time_t timer;
    static char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

class Thread {
    static int _signal;

  public:
    static void signalHandler(int signal) {
        printf("%s signalHandler called \n",timestamp());
    };
    void init() {
        struct sigaction sa;
        /* Install timer_handler as the signal handler for SIGVTALRM. */
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = &signalHandler;
        sigaction(SIGALRM, &sa, NULL);
    };
    uint32_t waitSignal(uint32_t msec) {
        struct timespec ts;
        ts.tv_sec = msec / 1000;
        ts.tv_nsec = (msec - ts.tv_sec * 1000) * 1000000;
        int erc=nanosleep(&ts, NULL);
        if (erc ) {
            printf("%s signal interrupted  %d \n",timestamp(),erc);
            return erc;
        }; 
        return 0;
    };
    void signal(int signal) {}
};

Thread thread;

void timer_handler(int signum) {
    static int count = 0;
    printf("%s timer expired %d times\n",timestamp(), ++count);
    thread.signal(1);
}

int linuxtest() {
    printf("%s started \n",timestamp());
    struct sigaction sa;
    struct itimerval timer;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 250000;
    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 250000;
    /* Start a virtual timer. It counts down whenever this process is
      executing. */
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    /* Do busy work. */
    thread.init();
    while (true) {
        int sign;
        if ((sign = thread.waitSignal(1000))) {
            printf("%s signal received %d \n",timestamp(), sign);
        } else {
            printf("%s normal timeout \n",timestamp());
        }
    };
}
#endif
