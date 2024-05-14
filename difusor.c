/*
** Fichero: difusor.c
** Autores:
** Javier Galante Gómez DNI 52417330P
** Manuel García Galante DNI 70957430S
** Usuario: i2417330
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>

#define TAM_MSG 512

static volatile bool end = false;

void SIGINTHandler(int signo)
{
    end = true;
}

int main(int argc, char *argv[])
{
    char multicast_addr[INET6_ADDRSTRLEN] = "ff15::66";
    char iface[64] = "eth0";
    int puerto = 4343;

    char msg[TAM_MSG] = "Hola que tal";
    int saltos = 15;
    int intervalo = 10;

    int sockfd, interfaz;
    struct sockaddr_in6 servaddr;

    if (argc == 7)
    {
        strncpy(msg, argv[1], TAM_MSG);
        strncpy(multicast_addr, argv[2], INET6_ADDRSTRLEN);
        strncpy(iface, argv[3], 64);
        puerto = atoi(argv[4]);
        saltos = atoi(argv[5]);
        intervalo = atoi(argv[6]);
    }

    printf("Parámetros:\n\tMensaje: %s\n\tDestino Multicast: %s\n\tInterfaz: %s\n\tPuerto: %d\n\tSaltos: %d\n\tIntervalo: %d\n",
           msg, multicast_addr, iface, puerto, saltos, intervalo);

    signal(SIGINT, SIGINTHandler);

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error en socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(puerto);
    if (inet_pton(AF_INET6, multicast_addr, &servaddr.sin6_addr) <= 0)
    {
        perror("inet_pton error");
        close(sockfd);
        exit(1);
    }

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &saltos, sizeof(saltos)) < 0)
    {
        perror("setsockopt IPV6_MULTICAST_HOPS error");
        close(sockfd);
        exit(1);
    }

    interfaz = if_nametoindex(iface);
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &interfaz, sizeof(interfaz)) < 0)
    {
        perror("setsockopt IPV6_MULTICAST_IF error");
        close(sockfd);
        exit(1);
    }

    while (!end)
    {
        sleep(intervalo);
        if (sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("sendto error");
        }
        else
        {
            printf("Mensaje enviado: %s\n", msg);
        }
    }
    close(sockfd);
    return 0;
}
