#include "ping.h"

int ping(const char* hostname)
{
    size_t size = SOC_RCV_MAX_BUF_SIZE;
    sigset_t sig_mask;

    socklen_t addr_from_len;

    // Structures for handling Internet addresses
    hostent_t* hosts_db_entry;

    char recv_buf[SOC_RCV_MAX_BUF_SIZE];
    int bytes_received;
    timeval_t tval;

    // PF_INET
    if ((sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("ping: socket");
        exit(ERR_CREATE_SOCKET);
    }
    setuid(geteuid()); // set EUID of the process addr_to RUID

    // addr_To prevent the receive buffer overflow:
    //   SOL_SOCKET: determines the socket-level manipulations
    //   SO_RCVBUF:  maximum socket receive buffer in bytes
    setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

    bzero(&addr_to, sizeof(addr_to));
    addr_to.sin_family = AF_INET; // AF_INET: address family for IPv4

    // Receive address
    hosts_db_entry = gethostbyname(hostname);
    if (hosts_db_entry == NULL) {
        perror("ping: gethostbyname");
        exit(ERR_HOST_NOT_FOUND);
    }
    addr_to.sin_addr = *((struct in_addr *) hosts_db_entry->h_addr);

    printf("PING %s (%s): %d data bytes\n", hostname,
      inet_ntoa(addr_to.sin_addr), (int) icmp_data_len);

    init_signal_handlers();
    init_interval_timer();

    pid = getpid();

    addr_from_len = sizeof(addr_from);
    while(1) {
        bytes_received = recvfrom(sd, recv_buf, sizeof(recv_buf), 0,
                            (struct sockaddr *) &addr_from, &addr_from_len);

        if (bytes_received < 0) {
            if (errno == EINTR)
                continue;
            perror("ping: recvfrom");
            continue;
        }
        gettimeofday(&tval, NULL);
        output(recv_buf, bytes_received, &tval);
    }

    if(close(sd) < 0) {
        perror("ping: close");
        exit(ERR_CREATE_SOCKET);
    }

    return EXIT_SUCCESS;
}

void output(char* recv_buf, int msglen, struct timeval* tval)
{
    int iplen;
    int icmplen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    double rtt;

    ip = (struct ip *) recv_buf; // IP header start
    iplen = ip->ip_hl << 2;      // IP header len
    icmp = (struct icmp *) (recv_buf + iplen); // ICMP header start

    icmplen = msglen - iplen;
    if (icmplen < ICMP_HEADER_LEN)
        fprintf(stderr, "icmplen (%d) < 8. msglen: %d, iplen: %d", icmplen, msglen, iplen);

    if (icmp->icmp_type == ICMP_ECHOREPLY) {

        if (icmp->icmp_id != pid) return; // request not for our query

        tvsend = (struct timeval *) icmp->icmp_data;
        sub_tval(tval, tvsend);

        // (round-trip time)  время оборота пакета
        rtt = tval->tv_sec * 1000.0 + tval->tv_usec / 1000.0;

        nreceived++;
        rtt_sum += rtt;
        if (rtt < rtt_min) rtt_min = rtt;
        if (rtt > rtt_max) rtt_max = rtt;

        printf("%d bytes addr_from %s: icmp_seq=%u, ttl=%d, time=%.2f ms\n", 
                icmplen, inet_ntoa(addr_from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
        ++nreceived;
    }
}

void init_interval_timer()
{
    itimerval_t interval_timer;

    // trigger itimer afer 1 ms..
    interval_timer.it_value.tv_sec = 0;
    interval_timer.it_value.tv_usec = 1;
    // ..every 1 s
    interval_timer.it_interval.tv_sec = 1;
    interval_timer.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &interval_timer, DONT_SAVE_OLD_VAL) < 0) {
        perror("pint: setitimer");
        exit(ERR_ITIMER);
    }
}

void init_signal_handlers()
{
    sigaction_t handler_action;

    // init with zero-valued bytes
    bzero(&handler_action, sizeof(handler_action));

    // Initialize signal mask
    sigemptyset(&handler_action.sa_mask);
    // specify a set of signals that aren’t permitted addr_to interrupt execution of
    // this handler.
    //
    // NOTE: signal that caused the handler addr_to be invoked is automatically added
    // addr_to the process signal mask, so a signal handler won't recursively
    // interrupt itself
    /* sigaddset(&handler_action.sa_mask, SIGQUIT); */
    handler_action.sa_handler = &catcher;

    // Bind signal handler
    sigaction(SIGALRM, &handler_action, DONT_SAVE_OLD_VAL);
    sigaction(SIGINT,  &handler_action, DONT_SAVE_OLD_VAL);
}


void pinger()
{
    struct icmp *icmp = (struct icmp *) send_buf;
    bzero(icmp, sizeof(struct icmp));

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_id = pid;
    icmp->icmp_seq = ntransmitted++;
    if (icmp_data_len >= sizeof(struct timeval))
        gettimeofday((struct timeval *) icmp->icmp_data, NULL);

    int icmplen = ICMP_HEADER_LEN + icmp_data_len;

    icmp->icmp_cksum = calc_in_cksum((u_int16_t *) icmp, icmplen);

    if (sendto(sd, send_buf, icmplen, 0,
               (struct sockaddr *) &addr_to, sizeof(addr_to)) < 0) {
        perror("sendto() failed");
        exit(ERR_ICMP_ECHO_SEND);
    }
}

void catcher(int signum)
{
    if (signum == SIGALRM) {
        pinger();
        return ;
    } else if (signum == SIGINT) {
        printf("\n--- %s ping statistics ---\n", inet_ntoa(addr_to.sin_addr));
        printf("%zu packets transmitted, %zu received", ntransmitted, nreceived);
        if (ntransmitted) {
            if (nreceived > ntransmitted)
                printf("-- somebody's printing up packets!");
            else
                printf("%d%% packet loss",
                       (int) (((ntransmitted - nreceived) * 100) / ntransmitted));
        }
        puts("");
        if (nreceived && icmp_data_len >= sizeof(timeval_t))
            printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n",
                   rtt_min, rtt_sum / nreceived, rtt_max);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}

