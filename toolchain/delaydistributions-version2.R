# ###########################################################################
#             Thomas Dreibholz's R Simulation Scripts Collection
#                 Copyright (C) 2005-2012 by Thomas Dreibholz
#
#               Author: Thomas Dreibholz, dreibh@iem.uni-due.de
# ###########################################################################


# ###### Identity distribution ##############################################
identityDelayDistribution <- function(currentNetwork, totalNetworks,
                                      componentType, currentComponent, totalComponents,
                                      variable, gamma, lambda)
{
   return(c("IdentityDelayDistribution",
            as.numeric(variable),
            as.numeric(variable)))
}


# ###### Uniform distribution ###############################################
uniformDelayDistribution <- function(currentNetwork, totalNetworks,
                                     componentType, currentComponent, totalComponents,
                                     variable, gamma, lambda)
{
   h <- as.numeric(variable) / 2
   r <- runif(1, as.numeric(variable) - h, as.numeric(variable) + h)
   return(c("UniformDelayDistribution",
            r,
            r))
}
