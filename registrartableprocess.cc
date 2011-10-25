/* $Id$
 * --------------------------------------------------------------------------
 *
 *              //===//   //=====   //===//   //=====  //   //      //
 *             //    //  //        //    //  //       //   //=/  /=//
 *            //===//   //=====   //===//   //====   //   //  //  //
 *           //   \\         //  //             //  //   //  //  //
 *          //     \\  =====//  //        =====//  //   //      //  Version V
 *
 * ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
 *
 * Copyright (C) 2003-2012 by Thomas Dreibholz
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
 * Contact: dreibh@iem.uni-due.de
 */

#include <omnetpp.h>
#include "messages_m.h"

#include "utilities.h"
#include "handlespacemanagementwrapper.h"


class RegistrarTableProcess : public cSimpleModule
{
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage* msg);


   // ====== Methods =========================================================
   void addStaticRegistrar(const unsigned int address);
   void handleRegistrarHuntRequest(RegistrarHuntRequest* msg);


   // ====== Variables =======================================================
   private:
   cPeerList* RegistrarTable;
   opp_string Description;
};

Define_Module(RegistrarTableProcess);


// ###### Initialize ########################################################
void RegistrarTableProcess::initialize()
{
   RegistrarTable = new cPeerList(NULL, 0);
   OPP_CHECK(RegistrarTable);

   Description = format("RegistrarTableProcess at %u:%u> ",
                        getLocalAddress(this), RegistrarAnnouncePort);

   // ------ Create registrar table -----------------------------------------
   const char*  staticRegistrarsList = par("staticRegistrarsList");
   unsigned int registrarAddress;
   int          n;
   while(staticRegistrarsList[0] != 0x00) {
      if(sscanf(staticRegistrarsList, "%u%n", &registrarAddress, &n) == 1) {
         addStaticRegistrar(registrarAddress);
         staticRegistrarsList = (const char*)&staticRegistrarsList[n];
         while( (*staticRegistrarsList == ' ') ||
                (*staticRegistrarsList == ',') ) {
           staticRegistrarsList++;
         }
      }
      else {
         break;
      }
   }

   ev << Description;
   RegistrarTable->print();

   // ------ Bind to port ---------------------------------------------------
   BindMessage* msg = new BindMessage("Bind");
   msg->setPort(RegistrarAnnouncePort);
   send(msg, "toTransport");
}


// ###### Clean up ##########################################################
void RegistrarTableProcess::finish()
{
   delete RegistrarTable;
   RegistrarTable = NULL;
}


// ###### Add static registrar to registrar table ###########################
void RegistrarTableProcess::addStaticRegistrar(const unsigned int address)
{
   ServerInformationParameter serverInformationParameter;
   serverInformationParameter.setServerID(0);
   serverInformationParameter.setAddress(address);
   serverInformationParameter.setPort(RegistrarPort);

   cPeerListNode* node;
   OPP_CHECK(RegistrarTable->registerPeerListNode(serverInformationParameter, node) == RSPERR_OKAY);
}


// ###### Handle registrar hunt request #####################################
void RegistrarTableProcess::handleRegistrarHuntRequest(RegistrarHuntRequest* msg)
{
   RegistrarHuntResponse* response = new RegistrarHuntResponse;
   cPeerListNode* node = RegistrarTable->getRandomPeerListNode();
   if(node) {
      response->setRegistrarAddress(node->getAddress());
      ev << Description << "Selected registrar at "
         << response->getRegistrarAddress() << endl;
   }
   else {
      response->setRegistrarAddress(0);
      ev << Description << "No registrar available!" << endl;
   }
   // printf("Selection of PR at %u for module %u\n",response->getRegistrarAddress(),getId());
   send(response, "toUser");
}


// ###### Handle message ####################################################
void RegistrarTableProcess::handleMessage(cMessage* msg)
{
   ev << Description << "Received message \"" << msg->getName() << endl;
   if(dynamic_cast<RegistrarHuntRequest*>(msg)) {
      handleRegistrarHuntRequest((RegistrarHuntRequest*)msg);
   }
   else {
      handleUnexpectedMsg(msg);
   }
   delete msg;
}
