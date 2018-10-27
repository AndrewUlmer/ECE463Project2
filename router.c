#include "ne.h"
#include "router.h"
#define MAXLINE 1024

int main(int argc, char **argv) 
{    
    unsigned int routerID = atoi(argv[1]);
    char * hostname = argv[2];
    int ne_port = atoi(argv[3]);
    int router_port = atoi(argv[4]);
    int received_bytes;
    unsigned int len;
    struct pkt_INIT_REQUEST * init_request = malloc(sizeof(struct pkt_INIT_REQUEST));
    struct pkt_INIT_RESPONSE * init_response = malloc(sizeof(struct pkt_INIT_RESPONSE));
    int sockfd;
    struct sockaddr_in servaddr;

    init_request->router_id = ntohl(routerID);
    
    // Create file descriptor
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server info
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ne_port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    
    // Send INIT_REQUEST 
    sendto(sockfd, init_request, sizeof(struct pkt_INIT_REQUEST), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // Receive INIT_RESPONSE
    ntoh_pkt_INIT_RESPONSE(init_response);
    received_bytes = recvfrom(sockfd, init_response, sizeof(struct pkt_INIT_RESPONSE), MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    ntoh_pkt_INIT_RESPONSE(init_response);
    close(sockfd);
    
    // Initialize routing table
    InitRoutingTbl(init_response, routerID);

    return 0;
}