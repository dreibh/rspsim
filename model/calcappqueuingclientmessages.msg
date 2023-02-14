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
// Copyright (C) 2003-2023 by Thomas Dreibholz
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
// Contact: thomas.dreibholz@gmail.com


class CalcAppQueuingClientJob extends cOwnedObject
{
    @customize(true);
    unsigned int JobID;
    simtime_t    JobQueuedAt;
    simtime_t    JobStartedAt;
    simtime_t    JobAcceptedAt;
    double       JobCalculations;

    simtime_t    TotalJobGoodputTime;
    simtime_t    TotalJobFailoverTime;

    unsigned int ServerSelections;
    unsigned int Unreachables;
    unsigned int Failovers;
    unsigned int CalcAppRequests;
    unsigned int CalcAppCookieEchoes;
    unsigned int CalcAppAccepts;
    unsigned int CalcAppRejects;

    simtime_t    LastGoodputAt;
    double       LastGoodputCalculations;
}
