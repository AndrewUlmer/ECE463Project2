#include "ne.h"
#include "router.h"

struct route_entry routingTable[MAX_ROUTERS];
int NumRoutes;

void InitRoutingTbl (struct pkt_INIT_RESPONSE *InitResponse, int myID)
{
    int i = 0; // for loop var

    // Check for valid ID
    if(myID >= MAX_ROUTERS)
    {
        printf("\nID exceeds number of routers supported by system.\n");
        exit(1);
    }

    // Initialize number of routes in the routing table
    NumRoutes = InitResponse->no_nbr + 1;

    // Add self-route entry
    routingTable[0].dest_id = myID;
    routingTable[0].next_hop = myID;
    routingTable[0].cost = 0;

    // Iterate through routing table and add initial neighbor entries
    for(i = 1; i < NumRoutes; ++i)
    {
        routingTable[i].dest_id = InitResponse->nbrcost[i-1].nbr;
        routingTable[i].next_hop = routingTable[i].dest_id;
        routingTable[i].cost = InitResponse->nbrcost[i-1].cost;
    }

    return;
}


int UpdateRoutes(struct pkt_RT_UPDATE *RecvdUpdatePacket, int costToNbr, int myID)
{
    int i = 0; // for loop var
    int j = 0; // for loop var
    int is_changed = 0; // boolean updated if routing table changed
    unsigned char is_found = 0x00; // boolean updated if received entry is found in routing table
    unsigned int cost_to_sender = 0; // cost to the router that sent their routing table

    // Get cost to sender router (HASH TABLE WOULD BE BETTER)
    for(j = 0; j < NumRoutes; ++j)
    {
        if(routingTable[j].dest_id == RecvdUpdatePacket->sender_id)
        {
            cost_to_sender = routingTable[j].cost;
        }
    }

    // Iterate through received routing table
    for(i = 0; i < RecvdUpdatePacket->no_routes; ++i)
    {
        is_found = 0x00; // Reset is_found boolean to not found

        // For known route, check if better route is found
        // Iterate through routing table
        for(j = 0; j < NumRoutes; ++j)
        {
            // Check if current entry is received entry
            if(routingTable[j].dest_id == RecvdUpdatePacket->route[i].dest_id)
            {
                is_found = 0xFF;

                // if received entry results in better route, 
                // update entry, set is_changed to 1, go to next received entry
                // if(forced update || found better path with split horizon)
                if((routingTable[j].next_hop == RecvdUpdatePacket->sender_id) || 
                    (((cost_to_sender + RecvdUpdatePacket->route[i].cost) < routingTable[j].cost ) && myID != RecvdUpdatePacket->route[i].next_hop))
                {
                    routingTable[j].next_hop = RecvdUpdatePacket->sender_id; // next_hop is sender
                    routingTable[j].cost = cost_to_sender + RecvdUpdatePacket->route[i].cost; // cost is through sender
                    if(routingTable[j].cost > INFINITY)
                    {
                        routingTable[j].cost = INFINITY;
                    }

                    is_changed = 1;
                }

                break; // go to next received entry (stop iterating through my routing table)
            }
        }

        // For unknown route, increment NumRoutes, add entry
        if(is_found == 0x00)
        {
            ++NumRoutes;
            routingTable[NumRoutes-1].dest_id = RecvdUpdatePacket->route[i].dest_id;
            routingTable[NumRoutes-1].next_hop = RecvdUpdatePacket->route[i].next_hop;
            routingTable[NumRoutes-1].cost = cost_to_sender + RecvdUpdatePacket->route[i].cost;
            is_changed = 1;
        }
    }

    return(is_changed);
}

void ConvertTabletoPkt(struct pkt_RT_UPDATE *UpdatePacketToSend, int myID)
{
    int i = 0; // for loop var

    UpdatePacketToSend->sender_id = myID;
    UpdatePacketToSend->no_routes = NumRoutes;

    // memcopy would probably be better
    for(i = 0; i < NumRoutes; ++i)
    {
        UpdatePacketToSend->route[i].dest_id = routingTable[i].dest_id;
        UpdatePacketToSend->route[i].next_hop = routingTable[i].next_hop;
        UpdatePacketToSend->route[i].cost = routingTable[i].cost;
    }
}

void PrintRoutes (FILE* Logfile, int myID)
{
    int i = 0; // for loop var

    fprintf(Logfile, "Routing Table:");

    for(i = 0; i < NumRoutes; ++i)
    {
        fprintf(Logfile, "\nR%d -> R%d: R%d, %d", myID, routingTable[i].dest_id, routingTable[i].next_hop, routingTable[i].cost);
    }

    fflush(Logfile);
}

void UninstallRoutesOnNbrDeath(int DeadNbr)
{
    int i = 0; // for loop var

    for(i = 0; i < NumRoutes; ++i)
    {
        if(routingTable[i].next_hop == DeadNbr)
        {
            routingTable[i].cost = INFINITY;
        }
    }
}