# ###########################################################################
#             Thomas Dreibholz's R Simulation Scripts Collection
#                 Copyright (C) 2005-2012 by Thomas Dreibholz
#
#               Author: Thomas Dreibholz, dreibh@iem.uni-due.de
# ###########################################################################



# ###### Identity distribution ##############################################
workloadIdentityDistribution <- function(currentBlock, totalBlocks,
                                         currentElement, totalElements,
                                         variable, gamma, lambda)
{
   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadIdentityDistribution: Check parameters!")
   }
   return(c("Identity",
            variable,
            variable))
}


# ###### Identity distribution ##############################################
workloadExponentialRandomizedDistribution <- function(currentBlock, totalBlocks,
                                                      currentElement, totalElements,
                                                      variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadExponentialRandomizedDistribution: Check parameters!")
   }
   return(c("RandNegExp",
            sprintf("exponential(%f)", variable),
            rexp(1, 1 / variable)))
}


# ###### Uniform randomized distribution ####################################
workloadUniformRandomizedDistribution <- function(currentBlock, totalBlocks,
                                                  currentElement, totalElements,
                                                  variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (poolUserServiceJobIntervalGamma < 0.0) ||
      (variable <= 0.0)) {
      stop("poolUserServiceJobIntervalUniformRandomizedDistribution: Check parameters!")
   }

   low <- variable
   high <- variable
   if(poolUserServiceJobIntervalGamma != 0) {
      low  <- variable - (variable * poolUserServiceJobIntervalGamma)
      high <- variable + (variable * poolUserServiceJobIntervalGamma)
   }
   if(low < 0) {
      cat("WARNING: poolUserServiceJobIntervalUniformRandomizedDistribution: low < 0. Setting it to 0.\n")
   }

   return(c("RandUniform",
            sprintf("uniform(%f,%f)", low, high),
            runif(1, min=low, max=high)))
}


# ###### Normalized linear distribution #####################################
workloadLinearDistribution <- function(currentBlock, totalBlocks,
                                       currentElement, totalElements,
                                       variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadLinearDistribution: Check parameters!")
   }
   if(gamma < 1.0) {
      stop("workloadLinearDistribution: gamma must be >= 1.0!")
   }

   minCapacity <- variable / (1 + ((gamma - 1) / totalElements))
   maxCapacity <- gamma * minCapacity

   totalCapacity <- variable * totalElements
   baseCapacity <- minCapacity * totalElements
   varCapacity <- totalCapacity - baseCapacity

   blocks <- (totalElements * (totalElements - 1)) / 2
   blockCapacity <- varCapacity / blocks

   serverCapacity <- (currentElement - 1) * blockCapacity + (baseCapacity / totalElements)

   return(c("Linear",
            serverCapacity,
            serverCapacity))
}


# ###### Non-normalized linear distribution #################################
workloadNonNormalizedLinearDistribution <- function(currentBlock, totalBlocks,
                                                    currentElement, totalElements,
                                                    variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadNonNormalizedLinearDistribution: Check parameters!")
   }
   if(gamma < 1.0) {
      stop("workloadNonNormalizedLinearDistribution: gamma must be >= 1.0!")
   }

   serverCapacity <- gamma * variable
   if(totalElements > 1) {
      totalCapacity <- gamma * totalElements * variable
      slowServerCapacity <- totalElements * variable
      totalAdditionalCapacity <- totalCapacity - slowServerCapacity


      maxStep <- (2 * totalAdditionalCapacity) / (totalElements - 1)
      m <- maxStep / totalElements
      serverCapacity <- variable + m * (currentElement - 1)
   }

   return(c("NNLinear",
             serverCapacity,
             serverCapacity))
}


# ###### Normalized fast server distribution ################################
workloadFastServersDistribution <- function(currentBlock, totalBlocks,
                                            currentElement, totalElements,
                                            variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadFastServersDistribution: Check parameters!")
   }
   if(gamma < 1.0) {
       stop("workloadFastServersDistribution: gamma must be >= 1.0!")
   }
   if((lambda < 0.0) || (lambda > 1.0)) {
       stop("workloadFastServersDistribution: lambda is not in valid range [0,1]!")
   }

   fastServers <- round(lambda * totalElements)
   slowServers <- totalElements - fastServers
   totalServers <- fastServers + slowServers
   if(totalServers != totalElements) {
      stop("workloadFastServersDistribution: server amount calculation error!")
   }

   fastServersFraction <- fastServers * gamma
   slowServersFraction <- slowServers
   totalFraction <- fastServersFraction + slowServersFraction

   totalCapacity <- variable * totalElements
   fastServersCapacity <- round(totalCapacity * (fastServersFraction / totalFraction))
   slowServersCapacity <- round(totalCapacity * (slowServersFraction / totalFraction))

   if(currentElement <= fastServers) {
      serverCapacity <- fastServersCapacity / fastServers
   }
   else {
      serverCapacity <- slowServersCapacity / slowServers
   }
   return(c("FastServers",
             serverCapacity,
             serverCapacity))
}


# ###### Non-normalized fast server distribution ############################
workloadNonNormalizedFastServersDistribution <- function(currentBlock, totalBlocks,
                                                         currentElement, totalElements,
                                                         variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadNonNormalizedFastServersDistribution: Check parameters!")
   }
   if(gamma < 1.0) {
       stop("workloadNonNormalizedFastServersDistribution: gamma must be >= 1.0!")
   }
   if((as.numeric(lambda < 0.0)) ||
      (as.numeric(lambda) > 1.0)) {
       stop("workloadNonNormalizedFastServersDistribution: lambda is not in valid range [0,1]!")
   }

   fastServers <- round(as.numeric(lambda) * totalElements)
   slowServers <- totalElements - fastServers
   totalServers <- fastServers + slowServers
   if(totalServers != totalElements) {
      stop("workloadNonNormalizedFastServersDistribution: server amount calculation error!")
   }

   totalCapacity <- gamma * totalServers * variable
   slowServerCapacity <- totalServers * variable
   totalAdditionalCapacity <- totalCapacity - slowServerCapacity
   additionalCapacity <- 0
   if(fastServers > 0) {
      additionalCapacity <- totalAdditionalCapacity / fastServers
   }

   serverCapacity <- variable
   if(currentElement <= fastServers) {
      serverCapacity <- variable + additionalCapacity
   }
   return(c("NNFastServers",
             serverCapacity,
             serverCapacity))
}


# ###### Uniformly randomized distribution ##################################
workloadUniformRandomizedDistribution <- function(currentBlock, totalBlocks,
                                                  currentElement, totalElements,
                                                  variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadUniformRandomizedDistribution: Check parameters!")
   }
   if(gamma < 1.0) {
       stop("workloadUniformRandomizedDistribution: gamma must be >= 1.0!")
   }

   minCapacity <- variable / (1 + ((gamma -1) / 2))
   maxCapacity <- gamma * minCapacity

   return(c("RandUniform",
            sprintf("uniform(%f,%f)", minCapacity, maxCapacity),
            runif(1, minCapacity, maxCapacity)))
}


# ###### Truncated normal random variable (>= 0) ############################
rtruncnorm <- function(n, mean, sd)
{
   result <- rnorm(n, mean, sd)
   while(result < 0) {
      result <- rnorm(n, mean, sd)
   }
   return(result)
}

# ###### Truncated normally randomized distribution #########################
workloadTruncnormalRandomizedDistribution <- function(currentBlock, totalBlocks,
                                                      currentElement, totalElements,
                                                      variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("workloadTruncnormalRandomizedDistribution: Check parameters!")
   }
   if((gamma > variable) || (gamma < 0)) {
       stop("workloadTruncnormalRandomizedDistribution: gamma is not in valid range [0,1]!!")
   }

   return(c("RandTruncNormal",
            sprintf("truncnormal(%f,%f)", variable, gamma * variable),
            rtruncnorm(1, variable, gamma * variable)))
}
