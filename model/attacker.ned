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


import transportnode.ned;


simple AttackerProcess
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double          componentStartupDelay @unit(s);
        int             componentRuns;
        volatile double componentUptime @unit(s);
        volatile double componentDowntime @unit(s);
        string          componentStatusChanges;

        // ------ Attacker Parameters ---------------------------------------
        string          attackType;
        volatile double attackInterval @unit(s);
        string          attackTargetRegistrarsList;
        volatile double attackReportUnreachableProbability;
        string          attackTargetPoolHandle;
        string          attackTargetPolicy;
        double          attackTargetPolicyLoadDegradation;
        double          attackTargetPolicyWeight;
        bool          attackRandomizeIdentifier;
        bool          attackAnswerKeepAlive;
    gates:
        output toTransport;
        input fromTransport;
}


module Attacker
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double componentStartupDelay @unit(s);

        @display("bgb=369,256");
    gates:
        output toNetwork;
        input fromNetwork;
    submodules:
        transportNode: TransportNode {
            parameters:
                interfaceAddress;
                @display("i=block/dispatch;p=194,177");
            gates:
                toApplication[1];
                fromApplication[1];
                fromNetwork[1];
                toNetwork[1];
        }
        attackerProcess: AttackerProcess {
            parameters:
                componentStartupDelay = componentStartupDelay;
                componentRuns = default(1);
                componentUptime = default(365d);
                componentDowntime = default(0s);
                componentStatusChanges = default("");

                attackType = default("HandleResolution");
                attackInterval = default(1.0s);
                attackTargetRegistrarsList = default("");
                attackReportUnreachableProbability = default(0.0);
                attackTargetPoolHandle = default("CalcAppPool");
                attackTargetPolicy = default("RoundRobin");
                attackTargetPolicyLoadDegradation = default(0);
                attackTargetPolicyWeight = default(4294967295.0);
                attackRandomizeIdentifier = default(false);
                attackAnswerKeepAlive = default(false);
                @display("p=194,81;i=block/control");
        }
    connections:
        fromNetwork --> transportNode.fromNetwork[0];
        transportNode.toNetwork[0] --> toNetwork;

        transportNode.toApplication[0] --> attackerProcess.fromTransport;
        attackerProcess.toTransport --> transportNode.fromApplication[0];
}
