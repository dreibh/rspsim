#include <omnetpp.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <sstream>


class cReqDistFromFile : public cRandom
{
   protected:
   std::vector<std::pair<simetime_t, double> AvgReqsPerMinute;

   private:
   void copy(const cReqDistFromFile& other);

   public:
   cReqDistFromFile(cRNG* rng, const char* avgReqsPerMinuteFileName) : cRandom(rng) {
      createRequestSchedule(avgReqsPerMinuteFileName);
   }
   cReqDistFromFile(const char* name=nullptr, cRNG* rng=nullptr, const char* avgReqsPerMinuteFileName) : cRandom(rng) {
      createRequestSchedule(avgReqsPerMinuteFileName);
   }
   cReqDistFromFile(const cReqDistFromFile& other) : cRandom(other) {
      copy(other);
   }
   virtual cReqDistFromFile* dup() const override {
      return new cReqDistFromFile(*this);
   }
   cReqDistFromFile& operator=(const cReqDistFromFile& other);
   virtual std::string str() const override;

   void
   virtual double draw() const override;
};


void cReqDistFromFile::createRequestSchedule(const char* avgReqsPerMinuteFileName)
{
   AvgReqsPerMinute.reset();

   std::ifstream is(avgReqsPerMinuteFileName);
   if(is.is_open()) {
      std::string line;
      while(getline(is, line)) {
         std::stringstream ss(line);
         std::string token;
         std::vector<std::string> tuple;
         while(getline(ss, token, '\t')) {
            tuple.push_back(token);
         }

         std::size_t index;
         const simtime_t timeStamp = (double)std::stoul(tuple[0], &index);
         assert(index == tuple[0].size());
         const unsigned int requests = std::stoul(tuple[1], &index);
         assert(index == tuple[1].size());

         std::cout << timeStamp << "\t" << requests << "\n";

         avgReqsPerMinute.push_back(std::make_pair(timeStamp, requests));
      }
   }
   else {
      error("Failed to read " + std::string(avgReqsPerMinuteFileName));
   }
}


void cReqDistFromFile::copy(const cReqDistFromFile& other)
{
   avgReqsPerMinute = other.avgReqsPerMinute;
}


cReqDistFromFile& cReqDistFromFile::operator=(const cReqDistFromFile& other)
{
   if(this == &other) {
      return *this;
   }

   cRandom::operator=(other);
   copy(other);
   return *this;
}


std::string cReqDistFromFile::str() const
{
    std::stringstream out;
    out << "alpha=" << alpha << ", theta=" << theta;
    return out.str();
}

double cReqDistFromFile::draw() const
{
//     return omnetpp::gamma_d(rng, alpha, theta);
}


int main(int argc, char* argv[]) 
{
   std::ifstream is("requests.csv");
   if(is.is_open()) {
      std::string line;
      while(getline(is, line)) {
         std::stringstream ss(line);
         std::string token;
         std::vector<std::string> tuple;
         while(getline(ss, token, '\t')) {
            tuple.push_back(token);
         }

         std::size_t index;
         const double timeStamp = (double)std::stoul(tuple[0], &index);
         assert(index == tuple[0].size());
         const unsigned int requests = std::stoul(tuple[1], &index);
         assert(index == tuple[1].size());

         std::cout << timeStamp << "\t" << requests << "\n";

         avgReqsPerMinute.push_back(std::make_pair(timeStamp, requests));
      }
   }
   return 0;
}
