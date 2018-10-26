#include "ne.h"
#include "router.h"
#define MAXLINE 1024

int main(int argc, char **argv) {
    
    // Grab the command line arguments
    unsigned int routerID = atoi(argv[1]);
    struct pkt_INIT_REQUEST *test = malloc(sizeof(struct pkt_INIT_REQUEST));
    test->router_id = ntohl(routerID);
    char *hostname = argv[2];
    int ne_port = atoi(argv[3]);
    int router_port = atoi(argv[4]);
    int n, len;    

    // Setup UDP connection with network emulator
    int sockfd;
    struct pkt_INIT_RESPONSE *buffer = malloc(sizeof(struct pkt_INIT_RESPONSE));

    // Continue setup
    struct sockaddr_in servaddr;
    
    // Create file descriptor
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Fill in server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ne_port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    

    // Send INIT_REQUEST 
    sendto(sockfd, test, sizeof(struct pkt_INIT_REQUEST), 
            MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    
    // Get INIT_RESPONSE
    ntoh_pkt_INIT_RESPONSE(buffer);
    n = recvfrom(sockfd, buffer, sizeof(struct pkt_INIT_RESPONSE), 
            MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    ntoh_pkt_INIT_RESPONSE(buffer);
    printf("# of directly connected neighbors: %d\n", buffer->no_nbr);
    close(sockfd);
    
    // Startup - Sends INIT_REQUEST for each router to Emulator (only router-id)
   

    // Take INIT_RESPONSE returned from network emulator and update routing table using InitRoutingTbl()

        
    // InitRoutingTbl(InitResponse, routerID);


    return 0;

}
