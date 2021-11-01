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


import transportnode.ned;
import registrartableprocess.ned;
import poolelementasapprocess.ned;


simple CalcAppServerProcess
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double          componentStartupDelay @unit(s);
        int             componentRuns;
        volatile double componentUptime @unit(s);
        volatile double componentDowntime @unit(s);
        string          componentStatusChanges;
        volatile double componentCleanShutdownProbability;

        // ------ Service Parameters ----------------------------------------
        double          serviceCapacity;
        double          serviceMaxJobs;
        double          serviceMinCapacityPerJob;
        double          serviceRejectProbability;

        // ------ Selection Parameters --------------------------------------
        string          selectionPolicy;
        double          selectionPolicyWeight;
        double          selectionPolicyLoadDegradation;
        double          selectionPolicyLoadDPF;
        double          selectionPolicyWeightDPF;
        double          selectionPolicyUpdateThreshold;
        bool            selectionReregisterImmediatelyOnUpdate;

        // ------ Service Parameters ----------------------------------------
        volatile int    servicePoolElementIdentifier;
        string          servicePoolHandle;
        double          serviceRegistrationLife @unit(s);
        volatile double serviceJobKeepAliveInterval @unit(s);
        double          serviceJobKeepAliveTimeout @unit(s);
        volatile double serviceCookieMaxTime @unit(s);
        volatile double serviceCookieMaxCalculations;

    gates:
        output toASAP;
        output toTransport;
        input fromASAP;
        input fromTransport;
}


module CalcAppPoolElement
{
    parameters:
        // ------ Component Parameters --------------------------------------
        double componentStartupDelay @unit(s);

        @display("bgb=338,230");
    gates:
        output toNetwork;
        input fromNetwork;

    submodules:
        transportNode: TransportNode {
            parameters:
                interfaceAddress;
                @display("i=block/dispatch;p=194,177");
            gates:
                toApplication[3];
                fromApplication[3];
                fromNetwork[1];
                toNetwork[1];
        }
        poolElementASAP: PoolElementASAPProcess {
            parameters:
                asapRegistrationTimeout = default(5s);
                asapDeregistrationTimeout = default(5s);
                asapMaxRegistrationAttempts = default(3);
                asapServerHuntRetryDelay = default(100ms);
                @display("i=block/cogwheel;p=84,53");
        }
        registrarTable: RegistrarTableProcess {
            parameters:
                staticRegistrarsList = default("");
                @display("p=84,131;i=block/table2");
        }
        calcAppServer: CalcAppServerProcess {
            parameters:
                componentStartupDelay = componentStartupDelay;
                componentRuns = default(1);
                componentUptime = default(365d);
                componentDowntime = default(0s);
                componentStatusChanges = default("");
                componentCleanShutdownProbability = default(1.0);

                serviceCapacity = default(1000000);
                serviceMaxJobs = default(4);
                serviceMinCapacityPerJob = default(250000);
                serviceRejectProbability = default(0.0);

                selectionPolicy = default("Random");
                selectionPolicyWeight = default(1);
                selectionPolicyLoadDegradation = default(0.25);
                selectionPolicyUpdateThreshold = default(0.0);
                selectionPolicyLoadDPF = default(0.0);
                selectionPolicyWeightDPF = default(0.0);
                selectionReregisterImmediatelyOnUpdate = default(true);

                serviceJobKeepAliveInterval = default(10s);
                serviceJobKeepAliveTimeout = default(5s);

                servicePoolElementIdentifier = default(uniform(1, 4294967295.0));
                servicePoolHandle = default("CalcAppPool");
                serviceRegistrationLife = default(300s);
                serviceCookieMaxTime = default(5s);
                serviceCookieMaxCalculations = default(5000000);
                @display("p=252,53;i=block/classifier");
        }
    connections:
        fromNetwork --> transportNode.fromNetwork[0];
        transportNode.toNetwork[0] --> toNetwork;

        transportNode.toApplication[0] --> poolElementASAP.fromTransport;
        poolElementASAP.toTransport --> transportNode.fromApplication[0];

        transportNode.toApplication[1] --> calcAppServer.fromTransport;
        calcAppServer.toTransport --> transportNode.fromApplication[1];

        registrarTable.toUser --> poolElementASAP.fromRegistrarTable;
        poolElementASAP.toRegistrarTable --> registrarTable.fromUser;

        transportNode.toApplication[2] --> registrarTable.fromTransport;
        registrarTable.toTransport --> transportNode.fromApplication[2];

        poolElementASAP.toApplication --> calcAppServer.fromASAP;
        calcAppServer.toASAP --> poolElementASAP.fromApplication;
}

