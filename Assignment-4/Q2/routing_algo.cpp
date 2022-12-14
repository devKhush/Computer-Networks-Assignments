#include "node.h"
#include <iostream>

using namespace std;

void printRT(vector<RoutingNode *> nd)
{
  /*Print routing table entries*/
  for (int i = 0; i < nd.size(); i++)
  {
    nd[i]->printTable();
  }
}

void routingAlgo(vector<RoutingNode *> nd)
{
  // Variable to check whether Routing tables have been converged or not
  bool converged = false;

// Repeat the LSR if not converged
main_loop:
  while (!converged)
  {
    // Assume all the routing tables entries have been converged
    converged = true;

    // Store the previous routing table entries of all the nodes
    // So that we can check the convergence of Routing tables entries later
    vector<vector<RoutingEntry>> oldRoutingEntry;
    for (RoutingNode *node : nd)
    {
      vector<RoutingEntry> oldEntry = node->getTable().tbl;
      vector<RoutingEntry> copyOldEntry;
      copy(oldEntry.begin(), oldEntry.end(), back_inserter(copyOldEntry));

      // Store the previous routing table entries
      oldRoutingEntry.push_back(copyOldEntry);
    }

    // Send routing tables entries to other nodes
    for (RoutingNode *node : nd)
    {
      node->sendMsg();
    }

    // Check routing tables of all the nodes
    for (int i = 0; i < nd.size(); ++i)
    {
      // Get the old routing table for that node
      vector<RoutingEntry> oldRoutingTable = oldRoutingEntry[i];
      // Get the new routing table for that node
      vector<RoutingEntry> newRoutingTable = nd[i]->getTable().tbl;

      // If Routing tables have been converged, then size should be same
      converged = converged && oldRoutingTable.size() == newRoutingTable.size();

      if (!converged)
        goto main_loop;

      for (int destination = 0; destination < newRoutingTable.size(); destination++)
      {
        // Routing tables entries will get converged if the previous and current Ip Interface are same
        converged = converged && (oldRoutingTable[destination].ip_interface == newRoutingTable[destination].ip_interface);

        // Routing tables entries will get converged if the previous and current nexthop are same
        converged = converged && (oldRoutingTable[destination].nexthop == newRoutingTable[destination].nexthop);

        // Start from while loop again if not converged
        if (!converged)
          goto main_loop;
      }
    }
  }

  /*Print routing table entries after routing algo converges */
  printf("Printing the routing tables after the convergence \n");
  printRT(nd);
}

void RoutingNode::recvMsg(RouteMsg *msg, int destinationCost)
{
  // your code here
  // Traverse the routing table in the message.
  // Check if entries present in the message table is closer than already present
  // entries.
  // Update entries.

  routingtbl *recvRoutingTable = msg->mytbl;
  for (RoutingEntry entry : recvRoutingTable->tbl)
  {
    // Check routing entry
    bool entryExists = false;
    for (int i = 0; i < mytbl.tbl.size(); ++i)
    {
      RoutingEntry myEntry = mytbl.tbl[i];
      // printf("i=%d, nodeRT.cost=%d, DV.cost=%d\n",i, myEntry.cost, entry.cost );
      if (myEntry.dstip == entry.dstip)
      {
        entryExists = true;
        // update existing entry
        if (myEntry.cost > entry.cost + destinationCost)
        {
          // update the entry cost
          myEntry.cost = entry.cost + destinationCost;
          myEntry.nexthop = msg->from;
          mytbl.tbl[i] = myEntry;
        }
      }
    }
    if (!entryExists)
    {
      // add the new entry
      RoutingEntry newEntry;
      newEntry.dstip = entry.dstip;
      newEntry.nexthop = msg->from;
      newEntry.ip_interface = msg->recvip;
      newEntry.cost = entry.cost + destinationCost;
      mytbl.tbl.push_back(newEntry);
    }
  }
}
