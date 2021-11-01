// $Id$
// --------------------------------------------------------------------------
//
//              //===//   //=====   //===//   //=====  //   //      //
//             //    //  //        //    //  //       //   //=/  /=//
//            //===//   //=====   //===//   //====   //   //  //  //
//           //   \\         //  //             //  //   //  //  //
//          //     \\  =====//  //        =====//  //   //      //  Version V
//
// ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
//
// Copyright (C) 2003-2012 by Thomas Dreibholz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Contact: dreibh@iem.uni-due.de


import link.ned;
import transportnode.ned;
import controller.ned;

import registrar.ned;
import poolelements.ned;
import poolusers.ned;
import attacker.ned;


module GammaSwitch
{
    parameters:
        int    numberOfBlocks;
        int    numberOfRegistrars;
        int    numberOfCalcAppPoolUsers;
        int    numberOfCalcAppPoolElements;
        int    numberOfAttackers;
        double interfaceUptime @unit(s);
        double interfaceDowntime @unit(s);
    gates:
        output toBlock[];
        output toRegistrar[];
        output toCalcAppPoolElement[];
        output toCalcAppPoolUser[];
        output toAttacker[];
        input fromBlock[];
        input fromRegistrar[];
        input fromCalcAppPoolElement[];
        input fromCalcAppPoolUser[];
        input fromAttacker[];
    submodules:
        transportNode: TransportNode {
            parameters:
                interfaceAddress;
                interfaceUptime = interfaceUptime;
                interfaceDowntime = interfaceDowntime;
                @display("p=134,64;i=abstract/switch");
            gates:
                fromNetwork[(((numberOfRegistrars+numberOfCalcAppPoolElements)+numberOfCalcAppPoolUsers)+numberOfAttackers)+numberOfBlocks];
                toNetwork[(((numberOfRegistrars+numberOfCalcAppPoolElements)+numberOfCalcAppPoolUsers)+numberOfAttackers)+numberOfBlocks];
                toApplication[0];
                fromApplication[0];
        }
    connections:
        for i=0..numberOfRegistrars-1 {
            transportNode.toNetwork[i] --> toRegistrar[i];
            fromRegistrar[i] --> transportNode.fromNetwork[i];
        }
        for i=0..numberOfCalcAppPoolElements-1 {
            transportNode.toNetwork[i+numberOfRegistrars] --> toCalcAppPoolElement[i];
            fromCalcAppPoolElement[i] --> transportNode.fromNetwork[i+numberOfRegistrars];
        }
        for i=0..numberOfCalcAppPoolUsers-1 {
            transportNode.toNetwork[(i+numberOfRegistrars)+numberOfCalcAppPoolElements] --> toCalcAppPoolUser[i];
            fromCalcAppPoolUser[i] --> transportNode.fromNetwork[(i+numberOfRegistrars)+numberOfCalcAppPoolElements];
        }
        for i=0..numberOfAttackers-1 {
            transportNode.toNetwork[((i+numberOfRegistrars)+numberOfCalcAppPoolElements)+numberOfCalcAppPoolUsers] --> toAttacker[i];
            fromAttacker[i] --> transportNode.fromNetwork[((i+numberOfRegistrars)+numberOfCalcAppPoolElements)+numberOfCalcAppPoolUsers];
        }
        for i=0..numberOfBlocks-1 {
            transportNode.toNetwork[(((i+numberOfRegistrars)+numberOfCalcAppPoolElements)+numberOfCalcAppPoolUsers)+numberOfAttackers] --> toBlock[i];
            fromBlock[i] --> transportNode.fromNetwork[(((i+numberOfRegistrars)+numberOfCalcAppPoolElements)+numberOfCalcAppPoolUsers)+numberOfAttackers];
        }
}



// ##########################################################################
// #### Networks                                                         ####
// ##########################################################################

module GammaScenarioNetwork
{
    parameters:
        int             numberOfRegistrars;
        int             numberOfCalcAppPoolUsers;
        int             numberOfCalcAppPoolElements;
        int             numberOfAttackers;

        double          networkLocalAddressBase;
        volatile double networkRegistrarStartTimeBase @unit(s);
        double          networkRegistrarStartTimeIncrement0 @unit(s);
        double          networkRegistrarStartTimeIncrement1 @unit(s);
        volatile double networkCalcAppPoolElementStartTimeBase @unit(s);
        double          networkCalcAppPoolElementStartTimeIncrement0 @unit(s);
        double          networkCalcAppPoolElementStartTimeIncrement1 @unit(s);
        volatile double networkCalcAppPoolUserStartTimeBase @unit(s);
        double          networkCalcAppPoolUserStartTimeIncrement0 @unit(s);
        double          networkCalcAppPoolUserStartTimeIncrement1 @unit(s);
        volatile double networkAttackerStartTimeBase @unit(s);
        double          networkAttackerStartTimeIncrement0 @unit(s);
        double          networkAttackerStartTimeIncrement1 @unit(s);

        @display("bgb=899,322;bgp=10,10");
    gates:
        output toLeftNetwork;
        output toRightNetwork;
        input fromLeftNetwork;
        input fromRightNetwork;

    submodules:

        switch: GammaSwitch {
            parameters:
                numberOfBlocks = 2;
                numberOfRegistrars = numberOfRegistrars;
                numberOfCalcAppPoolElements = numberOfCalcAppPoolElements;
                numberOfCalcAppPoolUsers = numberOfCalcAppPoolUsers;
                numberOfAttackers = numberOfAttackers;
                interfaceUptime = 100d;
                interfaceDowntime = 0s;
                @display("i=device/router;p=137,183");
            gates:
                toBlock[2];
                toRegistrar[numberOfRegistrars];
                toCalcAppPoolElement[numberOfCalcAppPoolElements];
                toCalcAppPoolUser[numberOfCalcAppPoolUsers];
                toAttacker[numberOfAttackers];
                fromBlock[2];
                fromRegistrar[numberOfRegistrars];
                fromCalcAppPoolElement[numberOfCalcAppPoolElements];
                fromCalcAppPoolUser[numberOfCalcAppPoolUsers];
                fromAttacker[numberOfAttackers];
        }
        registrarArray[numberOfRegistrars]: Registrar {
            parameters:
                componentStartupDelay = (index*networkRegistrarStartTimeIncrement1+networkRegistrarStartTimeIncrement0)+networkRegistrarStartTimeBase;
                @display("i=misc/globe;p=298,113,row");
        }
        calcAppPoolElementArray[numberOfCalcAppPoolElements]: CalcAppPoolElement {
            parameters:
                componentStartupDelay = (index*networkCalcAppPoolElementStartTimeIncrement1+networkCalcAppPoolElementStartTimeIncrement0)+networkCalcAppPoolElementStartTimeBase;
                @display("i=device/server2;p=138,50,row");
        }
        calcAppPoolUserArray[numberOfCalcAppPoolUsers]: CalcAppPoolUser {
            parameters:
                componentStartupDelay = (index*networkCalcAppPoolUserStartTimeIncrement1+networkCalcAppPoolUserStartTimeIncrement0)+networkCalcAppPoolUserStartTimeBase;
                @display("i=device/pc2;p=137,283,row");
        }
        attackerArray[numberOfAttackers]: Attacker {
            parameters:
                componentStartupDelay = (index*networkAttackerStartTimeIncrement1+networkAttackerStartTimeIncrement0)+networkAttackerStartTimeBase;
                @display("i=block/control;p=297,227,row");
        }
    connections:
        for i=0..numberOfRegistrars-1 {
            switch.toRegistrar[i] --> Link --> registrarArray[i].fromNetwork;
            registrarArray[i].toNetwork --> Link --> switch.fromRegistrar[i];
        }
        for i=0..numberOfCalcAppPoolElements-1 {
            switch.toCalcAppPoolElement[i] --> Link --> calcAppPoolElementArray[i].fromNetwork;
            calcAppPoolElementArray[i].toNetwork --> Link --> switch.fromCalcAppPoolElement[i];
        }
        for i=0..numberOfCalcAppPoolUsers-1 {
            switch.toCalcAppPoolUser[i] --> Link --> calcAppPoolUserArray[i].fromNetwork;
            calcAppPoolUserArray[i].toNetwork --> Link --> switch.fromCalcAppPoolUser[i];
        }
        for i=0..numberOfAttackers-1 {
            switch.toAttacker[i] --> Link --> attackerArray[i].fromNetwork;
            attackerArray[i].toNetwork --> Link --> switch.fromAttacker[i];
        }
        fromLeftNetwork --> switch.fromBlock[0];
        switch.toBlock[0] --> toLeftNetwork;
        fromRightNetwork --> switch.fromBlock[1];
        switch.toBlock[1] --> toRightNetwork;
}


module GammaScenario
{
    parameters:
        int    numberOfLANs;
        @display("bgb=300,174");
    submodules:
        controller: Controller {
            parameters:
                statisticsResetTime = default(-1.0s);
                statisticsWriteTime = default(1h);
                @display("p=46,56;i=abstract/penguin_l");
        }
        leftTerminator: Terminator {
            parameters:
                @display("p=147,64;i=misc/node_s");
        }
        rightTerminator: Terminator {
            parameters:
                @display("p=147,128;i=misc/node_s");
        }
        lan[numberOfLANs]: GammaScenarioNetwork {
            parameters:
                networkLocalAddressBase = 1000*(index+1);
                networkRegistrarStartTimeBase = 10.0s+index*30s;
                networkRegistrarStartTimeIncrement0 = 0s;
                networkRegistrarStartTimeIncrement1 = 10s;
                networkCalcAppPoolElementStartTimeBase = 300.0s;
                networkCalcAppPoolElementStartTimeIncrement0 = index*0.5s;
                networkCalcAppPoolElementStartTimeIncrement1 = 0s;
                networkCalcAppPoolUserStartTimeBase = 600.0s;
                networkCalcAppPoolUserStartTimeIncrement0 = 0s;
                networkCalcAppPoolUserStartTimeIncrement1 = 0s;
                networkAttackerStartTimeBase = 600.0s;
                networkAttackerStartTimeIncrement0 = 0s;
                networkAttackerStartTimeIncrement1 = 0s;

                numberOfRegistrars = default(1);
                numberOfCalcAppPoolElements = default(2);
                numberOfCalcAppPoolUsers = default(4);
                numberOfAttackers = default(0);
                @display("i=misc/cloud;p=252,95,row");
        }
    connections:
        for i=0..numberOfLANs-2 {
            lan[i].toRightNetwork --> Link --> lan[i+1].fromLeftNetwork if numberOfLANs>1;
            lan[i+1].toLeftNetwork --> Link --> lan[i].fromRightNetwork if numberOfLANs>1;
        }
        lan[0].toLeftNetwork --> leftTerminator.fromNetwork;
        leftTerminator.toNetwork --> lan[0].fromLeftNetwork;
        lan[numberOfLANs-1].toRightNetwork --> rightTerminator.fromNetwork;
        rightTerminator.toNetwork --> lan[numberOfLANs-1].fromRightNetwork;
}


network gammaScenario extends GammaScenario
{
    parameters:
}
