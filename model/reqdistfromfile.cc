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
 * Copyright (C) 2003-2025 by Thomas Dreibholz
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

#include <chrono>
#include <fstream>
#include <vector>
#include <cassert>
#include <sstream>
#include <string>
#include <iomanip>


// ###### Function ##########################################################
double uniformgamma(omnetpp::cRNG* rng, double mean, double gamma)
{
   const double low  = mean / (1 + ((gamma -1) / 2));
   const double high = gamma * low;
   return omnetpp::uniform(rng, low, high);
}


// ###### NED function ######################################################
omnetpp::cValue nedf_uniformgamma(omnetpp::cComponent* contextComponent, omnetpp::cValue argv[], int argc)
{
   if(!contextComponent) {
      throw omnetpp::cRuntimeError("No context component");
   }
   int rng = argc == 3 ? (int)argv[2] : 0;

   const double mean  = argv[0].doubleValueRaw();
   const double gamma = argv[1].doubleValueInUnit(argv[0].getUnit());

   const double low   = mean / (1 + ((gamma -1) / 2));
   const double high  = gamma * low;

   const double drawn = contextComponent->uniform(low, high, rng);

   // printf("%1.6f %1.6f mean=%1.6f gamma=%1.6f => %1.6f\n", low, high, mean, gamma, drawn);

   return omnetpp::cValue(drawn);
}

Define_NED_Function2(nedf_uniformgamma,
                     "quantity uniformgamma(quantity mean, quantity gamma, int rng?)",
                     "random/continuous",
                     "Returns a random number from the Uniform distribution, using gamma parameter");



// ###### Convert UTC time string to time point #############################
template <typename TimePoint> bool stringToTimePoint(
                                      const std::string& string,
                                      TimePoint&         timePoint,
                                      const char*        format = "%Y-%m-%d %H:%M:%S")
{
   // ====== Handle time in seconds granularity =============================
   std::istringstream iss(string);
   std::tm            tm = {};
   if(!(iss >> std::get_time(&tm, format))) {
      return false;
   }
   timePoint = TimePoint(std::chrono::seconds(std::mktime(&tm)));
   if(iss.eof()) {
      return true;
   }

   // ====== Handle fractional seconds ======================================
   double f = 0.0;
   if( (iss.peek() == '.') && (!(iss >> f)) )  {
      return false;
   }
   const size_t fseconds = f * std::chrono::high_resolution_clock::period::den / std::chrono::high_resolution_clock::period::num;
   timePoint += std::chrono::high_resolution_clock::duration(fseconds);

   return true;
}


// ###### Convert microseconds since the epoch to time point ################
template <typename TimePoint> unsigned long long timePointToMicroseconds(const TimePoint& timePoint)
{
   const std::chrono::microseconds microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(timePoint.time_since_epoch());
   return microseconds.count();
}



class cReqDistFromFile : public omnetpp::cRandom
{
   protected:
   std::vector<std::pair<double, double>> AvgReqsPerMinute;
   unsigned int                           Index;
   unsigned int                           Entries;
   int                                    AvgReqsColumn;

   private:
   void copy(const cReqDistFromFile& other);

   public:
   bool isInitialised() const { return Index > 0; }
   cReqDistFromFile(omnetpp::cRNG* rng,
                    const char*    avgReqsPerMinuteFileName,
                    const int      avgReqsColumn) : omnetpp::cRandom(rng) {
      createRequestSchedule(avgReqsPerMinuteFileName, avgReqsColumn);
   }
   cReqDistFromFile(const char*    name                     = nullptr,
                    omnetpp::cRNG* rng                      = nullptr,
                    const char*    avgReqsPerMinuteFileName = nullptr,
                    const int      avgReqsColumn            = -1) : cRandom(rng) {
      createRequestSchedule(avgReqsPerMinuteFileName, avgReqsColumn);
   }
   cReqDistFromFile(const cReqDistFromFile& other) : cRandom(other) {
      copy(other);
   }
   virtual cReqDistFromFile* dup() const override {
      return new cReqDistFromFile(*this);
   }
   cReqDistFromFile& operator=(const cReqDistFromFile& other);
   virtual std::string str() const override;

   void createRequestSchedule(const char* avgReqsPerMinuteFileName,
                              const int   avgReqsColumn);

   virtual double draw() const override;
};


// ###### Create request schedule ###########################################
void cReqDistFromFile::createRequestSchedule(const char* avgReqsPerMinuteFileName,
                                             const int   avgReqsColumn)
{
   AvgReqsPerMinute.clear();
   Index         = 0;
   Entries       = 0;
   AvgReqsColumn = avgReqsColumn;

   if(avgReqsPerMinuteFileName) {
      std::ifstream is(avgReqsPerMinuteFileName);
      if(is.is_open()) {
         std::string line;
         getline(is, line);   // Skip header

         double baseTimeStamp = -1.0;
         while(getline(is, line)) {
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tuple;
            while(getline(ss, token, ',')) {
               tuple.push_back(token);
            }
            if(tuple.size() < 2) {
               tuple.clear();
               ss = std::stringstream(line);
               while(getline(ss, token, '\t')) {
                  tuple.push_back(token);
               }
            }
            if(tuple.size() < 2) {
               throw omnetpp::cRuntimeError("Syntax error in %s\n", avgReqsPerMinuteFileName);
            }
            if(AvgReqsColumn < 0) {
               AvgReqsColumn = tuple.size() - 1;
            }
            else {
               if(AvgReqsColumn >= tuple.size()) {
                  printf("%d >= %d!  <%s>\n", AvgReqsColumn, (int)tuple.size() - 1, line.c_str());
                  throw omnetpp::cRuntimeError("Unexpected number of rows in %s\n", avgReqsPerMinuteFileName);
               }
            }

            std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
            if(stringToTimePoint<std::chrono::time_point<std::chrono::high_resolution_clock>>(
               tuple[0], timePoint, "%Y-%m-%dT%H:%M:%S+00:00") == false) {
               throw omnetpp::cRuntimeError("Bad time point in input file %s", avgReqsPerMinuteFileName);
            }
            const double timeStamp =
               timePointToMicroseconds<std::chrono::time_point<std::chrono::high_resolution_clock>>(timePoint) /
               1000000.0;

            std::size_t pos;
            const double requests = std::stod(tuple[AvgReqsColumn], &pos);
            assert(pos == tuple[AvgReqsColumn].size());

            if(baseTimeStamp < 0.0) {
               baseTimeStamp = timeStamp;
            }

            const double relTimeStamp = timeStamp - baseTimeStamp;
            if(relTimeStamp < 0.0) {
               throw omnetpp::cRuntimeError("Non-monotonous time stamp in %s", avgReqsPerMinuteFileName);
            }
            // std::cout << (unsigned long long)timeStamp << "\t" << relTimeStamp << "\t" << requests << "\n";

            AvgReqsPerMinute.push_back(std::make_pair(relTimeStamp, requests));
            Entries++;
         }
      }
      else {
         throw omnetpp::cRuntimeError("Failed to read %s", avgReqsPerMinuteFileName);
      }
   }
}


// ###### Copy object #######################################################
void cReqDistFromFile::copy(const cReqDistFromFile& other)
{
   AvgReqsPerMinute = other.AvgReqsPerMinute;
}


// ###### Copy object #######################################################
cReqDistFromFile& cReqDistFromFile::operator=(const cReqDistFromFile& other)
{
   if(this == &other) {
      return *this;
   }

   cRandom::operator=(other);
   copy(other);
   return *this;
}


// ###### Get information string ############################################
std::string cReqDistFromFile::str() const
{
   std::stringstream out;
   // out << ...
   return out.str();
}

// ###### Draw random number ################################################
// static double ST=0.0;
double cReqDistFromFile::draw() const
{
   const double now      = omnetpp::simTime().dbl();
   double       requests = -1.0;
   double       duration = 0.0;

   while(Index + 1 < Entries) {
      const double nextTimeStamp = AvgReqsPerMinute[Index + 1].first;
      if(now < nextTimeStamp) {
         requests = AvgReqsPerMinute[Index].second;
         duration = AvgReqsPerMinute[Index + 1].first - AvgReqsPerMinute[Index].first;
         break;
      }
      ((cReqDistFromFile*)this)->Index++;
   }

   if( (requests < 0.0) || (duration <= 0.0) ) {
      throw omnetpp::cRuntimeError("No time index for t=%1.6f. Not enough values provided for simulation duration!", now);
   }
   if( ! ( (AvgReqsPerMinute[Index].first <= now) &&
           (AvgReqsPerMinute[Index + 1].first > now)) ) {
      throw omnetpp::cRuntimeError("Unexpected time index. Something is wrong!");
   }

   const double reqsPerSecond    = requests / duration;
   const double interRequestTime = 1.0 / reqsPerSecond;

   // printf("%u:\t%1.3f @ %1.6f  R/s=%1.6f IRT=%1.6f\n", Index, requests, duration, reqsPerSecond, interRequestTime);

   // const double value = omnetpp::truncnormal(rng, interRequestTime, sqrt(interRequestTime));
   const double value = uniformgamma(rng, interRequestTime, 4.0);

   // printf("v=%1.6f\n", 50.0*value);
   return value;
}


// ###### Function ##########################################################
static omnetpp::cMersenneTwister myRNG;
static cReqDistFromFile          myRDFF(&myRNG, nullptr, -1);
double reqdistfromfile(const char* avgReqsPerMinuteFileName, const int avgReqsColumn)
{
   if(!myRDFF.isInitialised()) {
      myRDFF.createRequestSchedule(avgReqsPerMinuteFileName, avgReqsColumn);
   }
   return myRDFF.draw();
}


// ###### NED function ######################################################
omnetpp::cValue nedf_reqdistfromfile(omnetpp::cExpression::Context* context, omnetpp::cValue argv[], int argc)
{
    const std::string avgReqsPerMinuteFileName = argv[0].stringValue();
    const int         avgReqsColumn            = argv[1].intValue();
    return omnetpp::cValue(reqdistfromfile(avgReqsPerMinuteFileName.c_str(), avgReqsColumn));
}

Define_NED_Function2(nedf_reqdistfromfile,
                     "quantity reqdistfromfile(string avgReqsPerMinuteFileName, int column)",
                     "random/continuous",
                     "Draws inter-request time");



#if 0
#include <iostream>

using namespace omnetpp;

int main(int argc, char* argv[])
{
   cMersenneTwister rng;
   cReqDistFromFile r(&rng, "../toolchain/7day_task_req.csv");

   ST=0.0;
   for(unsigned int i = 0; i < 100; i++) {
      double v = r.draw();
      ST += 3600;
      std::cout << i+1 << "\t" << v << "\n";
   }

   return 0;
}
#endif
