#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <poll.h> 

struct __attribute__((packed)) socks4_request {
    unsigned char version;
    unsigned char command;
    unsigned short dest_port;
    unsigned int dest_ip;
};

int (*real_connect)(int, const struct sockaddr *, socklen_t);

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    real_connect = dlsym(RTLD_NEXT, "connect");

    if (addr->sa_family != AF_INET) {
        return real_connect(sockfd, addr, addrlen);
    }

    struct sockaddr_in *target_addr = (struct sockaddr_in *)addr;

    // 1. Connexion à Tor
    struct sockaddr_in tor_addr;
    memset(&tor_addr, 0, sizeof(tor_addr));
    tor_addr.sin_family = AF_INET;
    tor_addr.sin_port = htons(9050); 
    tor_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // On utilise la vraie connexion
    if (real_connect(sockfd, (struct sockaddr *)&tor_addr, sizeof(tor_addr)) < 0) {
        if (errno != EINPROGRESS) return -1;
    }

    // 2. Envoi de la requête SOCKS4
    struct socks4_request req;
    req.version = 4;
    req.command = 1;
    req.dest_port = target_addr->sin_port;
    req.dest_ip = target_addr->sin_addr.s_addr;

    write(sockfd, &req, 8);
    write(sockfd, "\0", 1); 

    // 3. ATTENDRE que Tor réponde (Crucial pour WSL)
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;
    
    // On attend jusqu'à 5 secondes que des données arrivent
    int poll_res = poll(&pfd, 1, 5000);
    if (poll_res <= 0) {
        fprintf(stderr, "[\033[31mErreur\033[0m] Tor ne répond pas (Timeout)\n");
        return -1;
    }

    // 4. Lecture de la réponse
    unsigned char resp[8];
    memset(resp, 0, 8);
    ssize_t n = read(sockfd, resp, 8);

    if (n >= 2 && resp[1] == 0x5A) {
        fprintf(stderr, "[\033[32mTOR\033[0m] Succès : %s\n", inet_ntoa(target_addr->sin_addr));
        return 0;
    }

    fprintf(stderr, "[\033[31mErreur\033[0m] Échec SOCKS (Octets lus: %ld, Code: 0x%02X)\n", n, resp[1]);
    errno = ECONNREFUSED;
    return -1;
}