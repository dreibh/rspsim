# ###########################################################################
#             Thomas Dreibholz's R Simulation Scripts Collection
#                 Copyright (C) 2005-2012 by Thomas Dreibholz
#
#               Author: Thomas Dreibholz, dreibh@iem.uni-due.de
# ###########################################################################


# Return input without any change
identityNumberDistribution <- function(currentBlock, totalBlocks,
                                       variable, gamma, lambda)
{
   return(c("IdentityNumberDistribution",
            variable,
            variable))
}


# Node distribution for a localized disaster
# gamma: LAN number of disaster
# lambda: PE amount factor for disaster LAN
# => PE amount of other LANs is equally increased to compensate
#    capacity loss in disaster LAN.
localizedDisasterDistribution <- function(currentBlock, totalBlocks,
                                          variable, gamma, lambda)
{
   disasterBlockNumber <- as.numeric(gamma)
   disasterBlockNodeNumberReduction <- as.numeric(lambda)
   variable <- as.numeric(variable)
   totalBlocks <- as.numeric(totalBlocks)

   otherBlocks <- totalBlocks - 1
   if(otherBlocks < 1) {
      stop("Number of blocks < 1!")
   }

   disasterBlockNodes <- floor(variable * disasterBlockNodeNumberReduction)
   if(disasterBlockNodes < 1) {
      disasterBlockNodes <- 1
   }
   otherBlockNodes <- ceiling(variable + (variable - disasterBlockNodes) / (totalBlocks - 1))

   if(currentBlock == disasterBlockNumber) {
      numberOfNodes <- disasterBlockNodes
   }
   else {
      totalNodes <- (totalBlocks) * variable

      for(i in seq(1, totalBlocks)) {
         if(i != currentBlock) {
            if(i != disasterBlockNumber) {
               totalNodes <- max(0, totalNodes - otherBlockNodes)
            }
            else {
               totalNodes <- max(0, totalNodes - disasterBlockNodes)
            }
         }
         else {
            numberOfNodes <- min(totalNodes, otherBlockNodes)
            totalNodes <- totalNodes - numberOfNodes
         }
      }

      if(totalNodes != 0) {
         stop("Total PEs != 0")
      }
   }

   return(c("LocalizedDisasterDistribution",
            numberOfNodes,
            numberOfNodes))
}
