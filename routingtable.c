#include "ne.h"
#include "router.h"


void InitRoutingTbl (struct pkt_INIT_RESPONSE *InitResponse, int myID)
{
    // Check for valid ID
    if(myID >= MAX_ROUTERS){
        printf("\nID exceeds number of routers supported by system.\n");
        exit(1);
    }

    // Create the empty routing table for this router
    struct route_entry *routingTable = malloc(sizeof(struct route_entry)*MAX_ROUTERS);
    memset(routingTable, 0, MAX_ROUTERS*sizeof(int));

    // Add in the values for the routing table
    int i = 0;
    while((i < MAX_ROUTERS) && ((InitResponse->nbrcost[i].nbr) < MAX_ROUTERS)) {
        // printf("neighbor = %d, cost = %d\n", InitResponse->nbrcost[i].nbr, InitResponse->nbrcost[i].cost);
        routingTable[i].dest_id = InitResponse->nbrcost[i].nbr;
        printf("dest_id = %d\n",routingTable[i].dest_id);
        i++;
    }
    
    return;
}
