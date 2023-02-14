# ###########################################################################
#             Thomas Dreibholz's R Simulation Scripts Collection
#                 Copyright (C) 2005-2023 by Thomas Dreibholz
#
#               Author: Thomas Dreibholz, thomas.dreibholz@gmail.com
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
