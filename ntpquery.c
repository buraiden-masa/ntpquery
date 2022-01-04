#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define NTP_SERVER_PORT 123
#define NTP_CLIENT_PORT 12345 // Set any values
#define JAN_1970 2208988800UL // 1900 - 1970 in seconds
#define FRAC 4294967295       // 2^32 - 1
#define TIMEOUT_SEC 10
#define TIMEOUT_USEC 0

int main(int argc, char *argv[]){
	int sock, ret;
	struct timeval timer;
	struct sockaddr_in ntpsrv;
	struct sockaddr_in client;
	fd_set fd, readfd;
	typedef struct {
		uint8_t li_vn_mode;       /* peer leap indicator */
		uint8_t stratum;          /* peer stratum */
		uint8_t poll;             /* peer pol interval */
		uint8_t precision;        /* peer clock precision */
		uint32_t rootDelay;       /* roundtrip delay to primary source */
		uint32_t rootDispersion;  /* dispersion to primary source */
		uint32_t refId;           /* reference id */
		uint32_t refTm_s;         /* last update time second */
		uint32_t refTm_f;         /* last update time fraction of a second */
		uint32_t origTm_s;        /* originate time stamp second */
		uint32_t origTm_f;        /* originate time stamp fraction of a second */
		uint32_t rxTm_s;          /* receive time stamp second */
		uint32_t rxTm_f;          /* receive time stamp fraction of a second */
		uint32_t txTm_s;          /* transmit time stamp second */
		uint32_t txTm_f;          /* transmit time stamp fraction of a second */
	} pkt;

	if(argc != 2){
		printf("Error: invalid argument\n");
		return 1;
	}

	/* NTP Server Settings */
	ntpsrv.sin_family = AF_INET;
	ntpsrv.sin_port = htons(NTP_SERVER_PORT);
	ntpsrv.sin_addr.s_addr = inet_addr(argv[1]);

	/* NTP Client Settings */
	client.sin_family = AF_INET;
	client.sin_port = htons(NTP_CLIENT_PORT);
	client.sin_addr.s_addr = INADDR_ANY;

	/* NTP packet Settings */
	pkt rpkt;
	pkt xpkt = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	memset(&xpkt, 0, sizeof(pkt));
	*((char *)&xpkt + 0 ) = 0x1b; // popular ntp messages

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){
		perror("socket");
		return 1;
	}

	ret = sendto(sock, (const char *)&xpkt, sizeof(xpkt), 0, (struct sockaddr *)&ntpsrv, sizeof(ntpsrv));
	if (ret < 0){
		perror("sendto");	
		close(sock);
		return 1;
	}

	/* Timeout setting */
	timer.tv_sec = TIMEOUT_SEC;
	timer.tv_usec = TIMEOUT_USEC;
	FD_ZERO(&readfd);
	FD_SET(sock, &readfd);

	ret = select(FD_SETSIZE, &readfd, (fd_set *)0, (fd_set *)0, &timer);
	if (ret < 0){
		perror("select");
		close(sock);
		return 1;
	} else if ( ret == 0 ){
		printf("Error: NTP server response timeout\n");
		close(sock);
		return 1;
	}

	if (read(sock, (char *)&rpkt, sizeof(pkt)) < 0) {
		printf("Error: reading from socket\n");
		close(sock);
		return 1;
	}

	/* Calc and print the result */
	time_t rxTm_s = (time_t)(ntohl(rpkt.rxTm_s) - JAN_1970) ;
	uint32_t rxTm_f = (ntohl(rpkt.rxTm_f) * 1e6 / FRAC );
	printf("[%s] rxTm: %d.%06lu .. %s", argv[1], rxTm_s, rxTm_f, ctime((const time_t *)&rxTm_s));

	time_t txTm_s = (time_t)(ntohl(rpkt.txTm_s) - JAN_1970) ;
	uint32_t txTm_f = (ntohl(rpkt.txTm_f) * 1e6 / FRAC );
	printf("[%s] txTm: %d.%06lu .. %s", argv[1], txTm_s, txTm_f, ctime((const time_t *)&txTm_s));

	close(sock);
	return 0;
}
