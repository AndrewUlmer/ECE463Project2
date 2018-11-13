#include "ne.h"
#include "router.h"
#include "time.h"
#include <string.h>
#define MAXLINE 1024

int main(int argc, char **argv) 
{    
    unsigned int routerID = atoi(argv[1]);
    char * hostname = argv[2];
    int ne_port = atoi(argv[3]);
    int router_port = atoi(argv[4]);
    FILE *fptr = NULL;
    char *output = malloc(sizeof("router") + sizeof(argv[1]) + sizeof(".log"));
    strcat(output,"router");
    strcat(output,argv[1]);
    strcat(output,".log");
    fptr = fopen(output,"w+");
    int retval;
    int is_changed = 0;
    int msec = 0;
    int i = 0; 
    int j = 0;
    int interval = 0; // haha
    int received_bytes;
    int UDPfd;
    int k = 0;
    struct timeval tv;
    unsigned int len;
    struct pkt_INIT_REQUEST * init_request = malloc(sizeof(struct pkt_INIT_REQUEST));
    struct pkt_INIT_RESPONSE * init_response = malloc(sizeof(struct pkt_INIT_RESPONSE));
    struct pkt_RT_UPDATE * RecvdUpdatePacket = malloc(sizeof(struct pkt_RT_UPDATE));
    struct sockaddr_in servaddr;
    fd_set rfds;

    init_request->router_id = ntohl(routerID);
    
    // Create file descriptor
    if((UDPfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
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
    sendto(UDPfd, 
           init_request, 
           sizeof(struct pkt_INIT_REQUEST), 
           MSG_CONFIRM, 
           (const struct sockaddr *)&servaddr, 
           sizeof(servaddr));

    // Receive INIT_RESPONSE
    ntoh_pkt_INIT_RESPONSE(init_response);
    received_bytes = recvfrom(UDPfd, 
                              init_response, 
                              sizeof(struct pkt_INIT_RESPONSE), 
                              MSG_WAITALL, 
                              (struct sockaddr *)&servaddr, 
                              &len);
    ntoh_pkt_INIT_RESPONSE(init_response);

    // Initialize routing table
    InitRoutingTbl(init_response, routerID);
   
    // Create empty update packet
    struct pkt_RT_UPDATE *SendUpdatePacket = malloc(sizeof(struct pkt_RT_UPDATE));
    ConvertTabletoPkt(SendUpdatePacket, routerID);

    // Create temp variables for neighbor info - is this ok?
    struct nbr_cost *nbrs = malloc(sizeof(struct nbr_cost) * init_response->no_nbr);
    for(i=0;i < init_response->no_nbr;i++) {
        nbrs[i] = init_response->nbrcost[i];
    }
    
    // Create another temp variable but the cost in this one holds the number of intervals since last update
    struct nbr_cost *nbrsInt = malloc(sizeof(struct nbr_cost) * init_response->no_nbr);
    for(i=0;i < init_response->no_nbr;i++) {
        nbrsInt[i].nbr = init_response->nbrcost[i].nbr;
        nbrsInt[i].cost = 0;
    }

    tv.tv_sec = UPDATE_INTERVAL;
    tv.tv_usec = 0;
    while(1) {
        // Set timer values

        // Set file descriptors
        FD_ZERO(&rfds);
        FD_SET(UDPfd, &rfds);
        
        // Check UDP Port on timer
        retval = select(UDPfd+1, &rfds, NULL, NULL, &tv);
        
        // Check for dead neighbors
        for(i = 0;i < init_response->no_nbr;i++) {
            printf("router: %d, interval: %d\n",nbrsInt[i].nbr, nbrsInt[i].cost);
            if(nbrsInt[i].cost > FAILURE_DETECTION) {
                UninstallRoutesOnNbrDeath(nbrsInt[i].nbr);
            }
        } 

        // If UDP file descriptor is set - receive and perform update
        if(FD_ISSET(UDPfd, &rfds)) {
            hton_pkt_RT_UPDATE(RecvdUpdatePacket);
            received_bytes = recvfrom(UDPfd, 
                                      RecvdUpdatePacket, 
                                      sizeof(struct pkt_RT_UPDATE), 
                                      MSG_WAITALL, 
                                      (struct sockaddr *)&servaddr, 
                                      &len);
            ntoh_pkt_RT_UPDATE(RecvdUpdatePacket);

            // If received update - reset cost for interval tracking array to 0
            for(i=0;i < init_response->no_nbr;i++) {
                if(nbrsInt[i].nbr == RecvdUpdatePacket->sender_id) {
                    nbrsInt[i].cost = 0;
                }
            }
            
            // Update routes
            is_changed = UpdateRoutes(RecvdUpdatePacket, 0, routerID);
            
            // If update is made - print the routes to log
            PrintRoutes(fptr, routerID);
        }
        else {
            interval++;
            if((interval == 5) &&  (is_changed == 0)) {
                fprintf(fptr,"Converged: %d",interval);                                    
                interval = 0;
            }
            
            tv.tv_sec = UPDATE_INTERVAL;
            tv.tv_usec = 0;
            
            // Once timer times out - send update
            ConvertTabletoPkt(SendUpdatePacket, routerID); // IDK
            for(j=0;j < init_response->no_nbr;j++) {    
                printf("sending update\n");
                SendUpdatePacket->dest_id = nbrs[j].nbr;
                hton_pkt_RT_UPDATE(SendUpdatePacket);
                sendto(UDPfd, 
                       SendUpdatePacket, 
                       sizeof(struct pkt_RT_UPDATE), 
                       MSG_CONFIRM, 
                       (const struct sockaddr *)&servaddr, 
                       sizeof(servaddr));
                ntoh_pkt_RT_UPDATE(SendUpdatePacket);
            }

            // Add one to the cost - the number of intervals a update has not been received
            for(i=0;i < init_response->no_nbr;i++) {
                nbrsInt[i].cost++;
            }
        }
        fprintf(fptr, "\n");    
    }
    return 0;
}
