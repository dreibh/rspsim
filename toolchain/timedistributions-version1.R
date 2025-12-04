# ###########################################################################
#             Thomas Dreibholz's R Simulation Scripts Collection
#                 Copyright (C) 2005-2026 by Thomas Dreibholz
#
#               Author: Thomas Dreibholz, thomas.dreibholz@gmail.com
# ###########################################################################



# ###### Identity distribution ##############################################
timeIdentityDistribution <- function(variable, gamma, lambda)
{
   return(c("Identity",
            variable,
            variable))
}


# ###### Identity distribution ##############################################
timeExponentialRandomizedDistribution <- function(variable, gamma, lambda)
{
   return(c("RandNegExp",
            sprintf("exponential(%ss)", variable),
            rexp(1, 1 / as.numeric(variable))))
}


# ###### MTBF from job size:PE capacity ratio distribution ##################
# Calculates MTBF as variable * job size:PE capacity ratio
# If gamma > 0, a fixed job size of gamma is used, e.g. gamma=1e7 calculations.
timePEmtbfFromMTBFJobSizePECapacityRatioDistribution <- function(variable, gamma, lambda)
{
   mtbfJobSizePECapacityRatio <- as.numeric(variable)
   mtbfBaseJobSize <- as.numeric(gamma)
   if(mtbfBaseJobSize <= 0) {
      mtbfBaseJobSize <- as.numeric(calcAppPoolUserServiceJobSizeVariable)
   }

   mtbf <- (mtbfJobSizePECapacityRatio * mtbfBaseJobSize) / as.numeric(calcAppPoolElementServiceCapacityVariable)

   # cat(sep="", gamma, " ", variable, " => mtbf=", mtbf, "\n")

   return(c("PEmtbfFromMTBFJobSizePECapacityRatio",
            sprintf("exponential(%fs)", mtbf),
            mtbf))
}
