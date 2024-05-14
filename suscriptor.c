/*
** Fichero: multicast.c
** Autores:
** Javier García Pechero  DNI 70906279Q
** Germán Francés Tostado DNI 70940996A
** Usuario: i0906279
*/

#include <sys/types.h>      /* basic system data types */
#include <sys/socket.h>     /* basic socket definitions */
#include <sys/time.h>       /* timeval{} for select() */
#include <time.h>           /* timespec{} for pselect() */
#include <netinet/in.h>     /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>      /* inet(3) functions */
#include <errno.h>
#include <fcntl.h>          /* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>       /* for S_xxx file mode constants */
#include <sys/uio.h>        /* for iovec{} and readv/writev */
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdbool.h>

#define TAM_MSG 512

bool end = false;

void SIGINTHandler(){ 
	fprintf(stdout, "\nSIGINTHandler\n");
	end = true;
}

int main(int argc, char *argv[]){

    char multicast_addr[INET6_ADDRSTRLEN] = "ff15::66";
    char iface[64] = "eth0";
    int puerto = 4343;
    int	listenfd;

    struct sockaddr_in6	servaddr, cliaddr;
    struct in6_addr resultipv6;
    struct ipv6_mreq group;

    struct sigaction sa = {.sa_handler = SIG_IGN};
    struct sigaction vec;

    char src_ip[INET6_ADDRSTRLEN];
    char msg[TAM_MSG];

    if (argc == 4){
        strncpy(multicast_addr, argv[1], 39);
        strncpy(iface, argv[2], 64);
        puerto = atoi(argv[3]);
    }

    fprintf(stdout, "Parámetros:\n\tDestino Multicast: %s\n\tInterfaz: %s\n\tPuerto: %d\n",
            multicast_addr, iface, puerto);

    vec.sa_handler = (void *) SIGINTHandler;
    vec.sa_flags = 0;
    if ( sigaction(SIGINT, &vec, (struct sigaction *) 0) == -1) {
        perror("No se ha podido registrar la señal SIGINT");
        exit(-1);
    }

	if ((listenfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Failed on creating socket");
        return 1;
    }

    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr   = in6addr_any;
    servaddr.sin6_port = htons(puerto);

	if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in6)) < 0)
    {
        perror("Failed on bind");
        if(close(listenfd) < -1) perror("couldn´t close the socket");
        return 1;
    }

	if (inet_pton(AF_INET6, multicast_addr, &resultipv6) <= 0) {
		perror("inet_pton: error al convertir la direcciOn\n");
		exit (-1);
    }

	group.ipv6mr_interface = if_nametoindex(iface);
    group.ipv6mr_multiaddr = resultipv6;

    if (setsockopt(listenfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(struct ipv6_mreq))) {
        perror("setsockopt: error al unirse al grupo");
        close(listenfd);
    }

    while(!end){
        recvfrom(listenfd, msg, TAM_MSG, 0, (struct sockaddr *)&cliaddr, NULL);
        if(!inet_ntop(AF_INET6, &cliaddr.sin6_addr, src_ip, sizeof(src_ip))){
            perror("No se pudo imprimir la direccion de origen");
        }
        fprintf(stdout, "FROM %s : %s\n", src_ip, msg);
    }

    return 0;
}
