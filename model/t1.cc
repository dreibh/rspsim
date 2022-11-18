#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <sstream>


std::vector<std::pair<double, unsigned int>> requestSchedule;


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

         requestSchedule.push_back(std::make_pair(timeStamp, requests));
      }
   }
   return 0;
}
