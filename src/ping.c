#include "ping.h"

int ping(const char* hostname)
{
    size_t size = SOC_RCV_MAX_BUF_SIZE;
    sigset_t sig_mask;
    socklen_t addr_from_len;
    hostent_t* hosts_db_entry; // Structures for handling Internet addresses
    char recv_buf[SOC_RCV_MAX_BUF_SIZE];
    int bytes_received;
    timeval_t timeout;

    // AF_INET: address family for IPv4
    if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("ping: socket");
        exit(ERR_CREATE_SOCKET);
    }
    setuid(geteuid()); // set EUID of the process addr_to RUID

    // To prevent the receive buffer overflow:
    //   SOL_SOCKET: determines the socket-level manipulations
    //   SO_RCVBUF:  maximum socket receive buffer in bytes
    setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

    // Receive address
    hosts_db_entry = gethostbyname(hostname);
    if (hosts_db_entry == NULL) {
        perror("ping: gethostbyname");
        exit(ERR_HOST_NOT_FOUND);
    }

    bzero(&addr_to, sizeof(addr_to));
    addr_to.sin_family = AF_INET; // AF_INET: address family for IPv4
    addr_to.sin_addr = *((struct in_addr *) hosts_db_entry->h_addr);


    // ICMP doesn't have PORT field, so we forced to identify packages by pid
    addr_from_len = sizeof(addr_from);
    pid = getpid();

    init_signal_handlers();
    init_interval_timer();

    printf("PING  %s (%s): %d data bytes\n", hostname,
            inet_ntoa(addr_to.sin_addr), (int) icmp_data_len);

    repeat_packets_receiving = true;
    while(repeat_packets_receiving) {
        bytes_received = recvfrom(sd, recv_buf, sizeof(recv_buf), 0,
                            (struct sockaddr *) &addr_from, &addr_from_len);

        if (bytes_received < 0) {
            if (errno == EINTR)
                continue;
            perror("ping: recvfrom");
            continue;
        }
        gettimeofday(&timeout, NULL);
        output(recv_buf, bytes_received, &timeout);
    }

    if(close(sd) < 0) {
        perror("ping: close");
        exit(ERR_CREATE_SOCKET);
    }

    return EXIT_SUCCESS;
}

int output(char* recv_buf, int msglen, struct timeval* timeout)
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
        // If multiple instances are running, kernel sends packets for each
        // raw soc. We need to ignore packets with another ID than current
        // pid.
        if (icmp->icmp_id != pid) {
            return -1;
        }

        if(icmplen - 8 >= sizeof(struct timeval)) {
            tvsend = (struct timeval *) icmp->icmp_data;
            sub_tval(timeout, tvsend);

            // round-trip time
            rtt = timeout->tv_sec * 1000.0 + timeout->tv_usec / 1000.0;

            rtt_sum += rtt;
            if (rtt < rtt_min) rtt_min = rtt;
            if (rtt > rtt_max) rtt_max = rtt;

            /* getnameinfo(&addr_from, sizeof(addr_from), host, 100, ); */

            printf("%d bytes from %s: icmp_seq=%u, ttl=%d, time=%.1f ms\n", 
                    icmplen, inet_ntoa(addr_from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
        } else {
            printf("%d bytes from %s: icmp_seq=%u, ttl=%d\n",
                   icmplen, inet_ntoa(addr_from.sin_addr),
                   icmp->icmp_seq, ip->ip_ttl);
        }

        ++nreceived;
    }
    return 0;
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
    /* sigaddset(&handler_action.sa_mask, SIGUSR1); */

    // Bind signal handler
    handler_action.sa_handler = &catcher;
    sigaction(SIGALRM, &handler_action, DONT_SAVE_OLD_VAL);
    sigaction(SIGINT,  &handler_action, DONT_SAVE_OLD_VAL);
}

void catcher(int signum)
{
    if (signum == SIGALRM) {
        pinger();
    } else if (signum == SIGINT) {
        finish();
    }
}

void pinger()
{
    struct icmp *icmp = (struct icmp *) send_buf;
    bzero(icmp, sizeof(struct icmp));

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0; // always zero
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

void finish()
{
    // inet_ntoa(): converts the Internet host address, given in network
    //              byte order, to a string in IPv4 dotted-decimal notation
    printf("\n--- %s ping statistics ---\n", inet_ntoa(addr_to.sin_addr));
    printf("%zu packets transmitted, %zu received", ntransmitted, nreceived);
    if (ntransmitted) {
        if (nreceived > ntransmitted) {
            printf("-- somebody's printing up packets!");
        } else {
            printf(", %d%% packet loss",
                    (int) (((ntransmitted - nreceived) * 100) / ntransmitted));
        }
    }
    puts("");
    if (nreceived) {
        printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n",
                rtt_min, rtt_sum / nreceived, rtt_max);
    }

    fflush(stdout);
    repeat_packets_receiving = false;
}

