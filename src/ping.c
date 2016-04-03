#include "ping.h"

int ping(const char* hostname)
{
    socket_t soc;
    size_t size = SOC_RCV_MAX_BUF_SIZE;
    sigset_t sig_mask;

    // Structures for handling INternet addresses
    sockaddr_in_t from;
    sockaddr_in_t to;
    hostent_t* hosts_db_entry;

    if ((soc = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("ping: socket");
        exit(ERR_CREATE_SOCKET);
    }
    setuid(geteuid()); // set EUID of the process to RUID

    // To prevent the receive buffer overflow:
    //   SOL_SOCKET: determines the socket-level manipulations
    //   SO_RCVBUF:  maximum socket receive buffer in bytes
    setsockopt(soc, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));


    bzero(&to, sizeof(to));
    to.sin_family = AF_INET; // AF_INET: address family for IPv4

    hosts_db_entry = gethostbyname(hostname);
    to.sin_addr = *((struct in_addr *) hosts_db_entry->h_addr);

    init_signal_handlers();

    if(close(soc) < 0) {
        perror("ping: close");
        exit(ERR_CREATE_SOCKET);
    }

    return EXIT_SUCCESS;
}

void init_signal_handlers()
{
    sigaction_t handler_action;

    // init with zero-valued bytes
    bzero(&handler_action, sizeof(handler_action));

    // Initialize signal mask
    sigemptyset(&handler_action.sa_mask);
    // specify a set of signals that aren’t permitted to interrupt execution of
    // this handler.
    //
    // NOTE: signal that caused the handler to be invoked is automatically added
    // to the process signal mask, so a signal handler won't recursively
    // interrupt itself
    sigaddset(&handler_action.sa_mask, SIGINT);
    sigaddset(&handler_action.sa_mask, SIGQUIT);
    handler_action.sa_handler = &catcher;

    // Bind signal handler
    sigaction(SIGALRM, &handler_action, DONT_SAVE_OLD_SIGACTION);
    sigaction(SIGINT,  &handler_action, DONT_SAVE_OLD_SIGACTION);
}

void catcher(int signum)
{
    printf("%d\n", signum);
}
