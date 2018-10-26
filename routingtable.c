#include "ne.h"
#include "router.h"

void InitRoutingTbl (struct pkt_INIT_RESPONSE *InitResponse, int myID)
{
    // Check for valid ID
    if(myID >= MAX_ROUTERS){
        printf("\nID exceeds number of routers supported by system.\n");
        exit(1);
    }

    // Create empty routing table
    struct route_entry routingTable[MAX_ROUTERS];

    return;
}
