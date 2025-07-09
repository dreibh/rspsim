/* --------------------------------------------------------------------------
 *
 *              //===//   //=====   //===//   //=====  //   //      //
 *             //    //  //        //    //  //       //   //=/  /=//
 *            //===//   //=====   //===//   //====   //   //  //  //
 *           //   \\         //  //             //  //   //  //  //
 *          //     \\  =====//  //        =====//  //   //      //  Version V
 *
 * ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
 *
 * Copyright (C) 2003-2024 by Thomas Dreibholz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: thomas.dreibholz@gmail.com
 */

#include <omnetpp.h>

#include "utilities.h"
#include "messages_m.h"
#include "statisticswriterinterface.h"
#include "abstractcontroller.h"
#include "statuschangelist.h"
#include "utilities.h"
#include "simpleredblacktree.h"


class SwitchingTableEntry
{
   public:
   SwitchingTableEntry(const unsigned int destinationAddress,
                       cGate*             outputGate);
   ~SwitchingTableEntry();
   static void switchingTableEntryPrintFunction(const void* a, FILE* fd);
   static int switchingTableEntryComparisonFunction(const void* a, const void* b);

   public:
   SimpleRedBlackTreeNode Node;
   unsigned int           DestinationAddress;
   cGate*                 OutputGate;
};


// ###### Constructor #######################################################
SwitchingTableEntry::SwitchingTableEntry(const unsigned int destinationAddress,
                                         cGate*             outputGate)
{
   simpleRedBlackTreeNodeNew(&Node);
   DestinationAddress = destinationAddress;
   OutputGate         = outputGate;
}


// ###### Clean up ##########################################################
SwitchingTableEntry::~SwitchingTableEntry()
{
   CHECK(!simpleRedBlackTreeNodeIsLinked(&Node));
   simpleRedBlackTreeNodeDelete(&Node);
}


// ###### Print function ####################################################
void SwitchingTableEntry::switchingTableEntryPrintFunction(const void* a, FILE* fd)
{
   // const SwitchingTableEntry* ste = (const SwitchingTableEntry*)a;
}


// ###### Comparison function ###############################################
int SwitchingTableEntry::switchingTableEntryComparisonFunction(const void* a,
                                                               const void* b)
{
   const SwitchingTableEntry* ste1 = (const SwitchingTableEntry*)a;
   const SwitchingTableEntry* ste2 = (const SwitchingTableEntry*)b;
   if(ste1->DestinationAddress < ste2->DestinationAddress) {
      return(-1);
   }
   else if(ste1->DestinationAddress > ste2->DestinationAddress) {
      return(1);
   }
   return(0);
};



class TransportNode : public StatisticsWriterInterface,
                      public cSimpleModule
{
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage* simplePacket);

   virtual void resetStatistics();
   virtual void writeStatistics();


   // ====== Methods =========================================================
   private:
   inline int returnGateID(int arrivalGateID) const;
   cGate* computeShortestPath(const unsigned int destinationAddress);
   cGate* findRoute(const unsigned int destinationAddress);
   simtime_t getUpDownTime(const bool up);


   // ====== Parameters ======================================================
   unsigned int       LocalAddress;
   StatusChangeList   ComponentStatusChanges;


   // ====== Variables =======================================================
   unsigned long long TotalCookies;
   unsigned long long TotalASAPPackets;
   unsigned long long TotalENRPPackets;

   cArray             PortMappingArray;
   int                ToAppGateArrayID;
   int                ToAppGateArraySize;
   int                FromAppGateArrayID;
   int                FromAppGateArraySize;
   SimpleRedBlackTree SwitchingTable;

   bool               HasInterfaceUp;
   cMessage*          InterfaceUpDownTimer;

   opp_string         Description;
};

Define_Module(TransportNode);



// ###### Initialize ########################################################
void TransportNode::initialize()
{
   // ------ Initialize variables -------------------------------------------
   LocalAddress = par("interfaceAddress");
   ComponentStatusChanges.setup(par("interfaceStatusChanges"));
   Description  = format("TransportNode at %u> ",
                         LocalAddress);

   PortMappingArray.setName("PortMappingArray");
   if(gateSize("toApplication") > 0) {
      const cGate* toAppGate = gate("toApplication",0);
      OPP_CHECK(toAppGate);
      ToAppGateArrayID   = toAppGate->getId();
      ToAppGateArraySize = toAppGate->size();
   }
   else {
      ToAppGateArrayID   = -1;
      ToAppGateArraySize =  0;
   }
   if(gateSize("fromApplication") > 0) {
      const cGate* fromAppGate = gate("fromApplication",0);
      OPP_CHECK(fromAppGate);
      FromAppGateArrayID   = fromAppGate->getId();
      FromAppGateArraySize = fromAppGate->size();
   }
   else {
      FromAppGateArrayID   = -1;
      FromAppGateArraySize =  0;
   }

   simpleRedBlackTreeNew(&SwitchingTable,
                             SwitchingTableEntry::switchingTableEntryPrintFunction,
                             SwitchingTableEntry::switchingTableEntryComparisonFunction);

   // ------ Prepare shutdown timer -----------------------------------------
   HasInterfaceUp = true;
   const simtime_t interfaceUptime = getUpDownTime(true);
   InterfaceUpDownTimer = new cMessage("InterfaceDownTimer");
   scheduleAt(simTime() + interfaceUptime, InterfaceUpDownTimer);
   EV << Description << "Interface is up for " << interfaceUptime << "s" << endl;

   // ------ Reset statistics -----------------------------------------------
   resetStatistics();
}


// ###### Clean up ##########################################################
void TransportNode::finish()
{
   SwitchingTableEntry* ste = (SwitchingTableEntry*)simpleRedBlackTreeGetFirst(&SwitchingTable);
   while(ste != NULL) {
      simpleRedBlackTreeRemove(&SwitchingTable, &ste->Node);
      delete ste;
      ste = (SwitchingTableEntry*)simpleRedBlackTreeGetFirst(&SwitchingTable);
   }
   simpleRedBlackTreeDelete(&SwitchingTable);
}


// ###### Reset statistics ##################################################
void TransportNode::resetStatistics()
{
   TotalCookies     = 0;
   TotalASAPPackets = 0;
   TotalENRPPackets = 0;
}


// ###### Write statistics ##################################################
void TransportNode::writeStatistics()
{
   recordScalar("TransportNode Total Cookies",      TotalCookies);
   recordScalar("TransportNode Total ASAP Packets", TotalASAPPackets);
   recordScalar("TransportNode Total ENRP Packets", TotalENRPPackets);
}


// ###### Get uptime or downtime ############################################
simtime_t TransportNode::getUpDownTime(const bool up)
{
   simtime_t time;
   if(up) {
      time = ComponentStatusChanges.expectedAction(simTime(), "down");
      if(time < 0.0) {
         time = (simtime_t)par("interfaceUptime");
      }
   }
   else {
      time = ComponentStatusChanges.expectedAction(simTime(), "up");
      if(time < 0.0) {
         time = (simtime_t)par("interfaceDowntime");
      }
   }
   return(time);
}


// ###### Get return gate ID for given arrival gate ID ######################
inline int TransportNode::returnGateID(int arrivalGateID) const
{
   const int index = arrivalGateID - FromAppGateArrayID;
   return(ToAppGateArrayID + index);
}


// ###### Compute output gate using shortest path algorithm #################
cGate* TransportNode::computeShortestPath(const unsigned int destinationAddress)
{
   cTopology topology;
   topology.extractByNedTypeName(cStringTokenizer("TransportNode").asVector());

   // Current node has only one output link -> routing is trivial!
   cTopology::Node* currentNode = topology.getNodeFor(this);
   if(currentNode->getNumOutLinks() == 1) {
      return(currentNode->getLinkOut(0)->getLocalGate());
   }

   // Find remote node by its destination address
   for(int i = 0;i < topology.getNumNodes();i++) {
      cTopology::Node* destinationNode          = topology.getNode(i);
      TransportNode*   destinationTransportNode = dynamic_cast<TransportNode*>(destinationNode->getModule());
      if((destinationTransportNode != NULL) &&
         (destinationTransportNode->LocalAddress == destinationAddress)) {
         // Compute shortest paths to destination node
         topology.calculateUnweightedSingleShortestPathsTo(destinationNode);
         return(currentNode->getPath(0)->getLocalGate());
      }
   }
   return(NULL);
}


// ###### Get output gate by trying switching table lookup first ############
cGate* TransportNode::findRoute(const unsigned int destinationAddress)
{
   SwitchingTableEntry  cmpEntry(destinationAddress, NULL);
   SwitchingTableEntry* found;
   found = (SwitchingTableEntry*)simpleRedBlackTreeFind(&SwitchingTable, &cmpEntry.Node);
   if(found) {
      return(found->OutputGate);
   }
   else {
      cGate* outputGate = computeShortestPath(destinationAddress);
      if(outputGate) {
         SwitchingTableEntry* newEntry = new SwitchingTableEntry(destinationAddress,
                                                                  outputGate);
         simpleRedBlackTreeInsert(&SwitchingTable, &newEntry->Node);
      }
      return(outputGate);
   }
}


// ###### Handle message ####################################################
void TransportNode::handleMessage(cMessage* msg)
{
   if(dynamic_cast<SimplePacket*>(msg) != NULL) {
      SimplePacket* simplePacket = (SimplePacket*)msg;
      if(HasInterfaceUp) {
         const int arrivalGateID = simplePacket->getArrivalGateId();

         // ====== Message from application layer to network ================
         if((arrivalGateID >= FromAppGateArrayID) &&
            (arrivalGateID < FromAppGateArrayID + FromAppGateArraySize)) {
            int i;
            for(i = 0;i < PortMappingArray.size();i++) {
               if((PortMappingArray[i]) &&
                  ((cPortMapping*)PortMappingArray[i])->getPort() == simplePacket->getSrcPort()) {
                  break;
               }
            }
            if(i == PortMappingArray.size()) {
               error("Source port %u is not bound to this application!", simplePacket->getSrcPort());
            }

            // Set source address
            simplePacket->setSrcAddress(LocalAddress);

            // ====== Statistics ============================================
            if(dynamic_cast<ASAPPacket*>(simplePacket)) {
               if((dynamic_cast<ASAPCookie*>(simplePacket) == NULL) &&
                  (dynamic_cast<ASAPCookieEcho*>(simplePacket) == NULL)) {
                  TotalASAPPackets++;
                  AbstractController* controller = AbstractController::getController();
                  if(controller) {
                     controller->GlobalASAPPackets++;
                  }
               }
               else if(dynamic_cast<ASAPCookie*>(simplePacket)) {
                  TotalCookies++;
                  AbstractController* controller = AbstractController::getController();
                  if(controller) {
                     controller->GlobalCookies++;
                  }
               }
            }
            else if(dynamic_cast<ENRPPacket*>(simplePacket)) {
               TotalENRPPackets++;
               AbstractController* controller = AbstractController::getController();
               if(controller) {
                  controller->GlobalENRPPackets++;
               }
            }
         }

         // ====== Message from network to local node =======================
         if(simplePacket->getDstAddress() == LocalAddress) {
            // ====== Forward message to application layer port =============
            if(simplePacket->getProtocol() != Ping) {
               int i;
               for(i = 0;i < PortMappingArray.size();i++) {
                  if((PortMappingArray[i]) &&
                     ((cPortMapping*)PortMappingArray[i])->getPort() == simplePacket->getDstPort()) {
                     send((cMessage*)simplePacket, ((cPortMapping*)PortMappingArray[i])->getGate());
                     return;   // Simply move packet, do not duplicate!
                  }
               }
               if(i == PortMappingArray.size()) {
                  EV << "Unknown destination port " << simplePacket->getDstPort();
               }
            }

            // ====== Ping protocol handling ================================
            else {
               SimplePacket* response = (SimplePacket*)simplePacket->dup();
               response->setProtocol(Pong);
               response->setSrcAddress(LocalAddress);
               response->setDstAddress(simplePacket->getSrcAddress());
               response->setSrcPort(0);
               response->setDstPort(simplePacket->getSrcPort());
               handleMessage(response);   // Route response!
            }
         }

         // ====== Message from network to other node =======================
         else {
            cGate* outputGate = findRoute(simplePacket->getDstAddress());
            if(outputGate) {
               if(simplePacket->getHopCount() > 0) {
                  simplePacket->setHopCount(simplePacket->getHopCount() - 1);
                  send((cMessage*)simplePacket, outputGate);
                  return;   // Simply move packet, do not duplicate!
               }
               else {
                  error("Node %u: HopCount of message to %u:%u is zero -> No more routing!",
                        LocalAddress, simplePacket->getDstAddress(), simplePacket->getDstPort());
               }
            }
            else {
               std::cout << *simplePacket << std::endl;
               error("Node %u: No route to destination %u:%u!",
                        LocalAddress, simplePacket->getDstAddress(), simplePacket->getDstPort());
            }
         }
      }
   }

   // ====== Up/Down timer ==================================================
   else if(msg == InterfaceUpDownTimer) {
      if(HasInterfaceUp) {
         HasInterfaceUp = false;
         const simtime_t interfaceDowntime = getUpDownTime(false);
         InterfaceUpDownTimer = new cMessage("InterfaceUpTimer");
         scheduleAt(simTime() + interfaceDowntime, InterfaceUpDownTimer);
         EV << Description << "Interface is down for " << interfaceDowntime << "s" << endl;
      }
      else {
         HasInterfaceUp = true;
         const simtime_t interfaceUptime = getUpDownTime(true);
         InterfaceUpDownTimer = new cMessage("InterfaceDownTimer");
         scheduleAt(simTime() + interfaceUptime, InterfaceUpDownTimer);
         EV << Description << "Interface is up for " << interfaceUptime << "s" << endl;
      }
   }

   // ====== Control message from application layer: Bind/Unbind ============
   else if(dynamic_cast<BindMessage*>(msg) != NULL) {
      BindMessage* bindMessage = (BindMessage*)(msg);
      for(int i = 0;i < PortMappingArray.size();i++) {
         if((PortMappingArray[i]) &&
            ((cPortMapping*)PortMappingArray[i])->getPort() == bindMessage->getPort()) {
            error("Node %u: Port %u is already bound!", LocalAddress, bindMessage->getPort());
            break;
         }
      }
      cPortMapping* portMapping = new cPortMapping;
      portMapping->setPort(bindMessage->getPort());
      portMapping->setGate(returnGateID(bindMessage->getArrivalGateId()));
      PortMappingArray.add(portMapping);
      EV << Description
         << "Node " << LocalAddress << " - Port " << portMapping->getPort()
         << " bound to gate " << portMapping->getGate() << endl;
   }
   else if(dynamic_cast<UnbindMessage*>(msg) != NULL) {
      UnbindMessage* unbindMessage = (UnbindMessage*)(msg);
      for(int i = 0;i < PortMappingArray.size();i++) {
         if((PortMappingArray[i]) &&
            ((cPortMapping*)PortMappingArray[i])->getPort() == unbindMessage->getPort()) {
            EV << Description
               << "Node " << LocalAddress << " - Unbinding port " << unbindMessage->getPort()
               << " from gate " << ((cPortMapping*)PortMappingArray[i])->getGate() << endl;
            delete PortMappingArray.remove(i);
            break;
         }
      }
   }

   // ====== Something went wrong ... =======================================
   else {
      error("Node %u: Unknown message \"%s\" received!", LocalAddress, msg->getName());
   }

   delete msg;
}
