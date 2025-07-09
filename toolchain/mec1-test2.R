# ###########################################################################
# Name:        mec1-test2.R
# Description: MEC/PMC performance of DPF policies
# ###########################################################################

source("simulate-version14.R")


# ====== Simulation Settings ================================================
simulationDirectory <- "mec1-test2"
simulationRuns <- 64
simulationDuration <- 120
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 3
simulationSummaryCompressionLevel <- 9
simulationSummarySkipList <- c()
# -------------------------------------
source("computation-pool.R")
# -------------------------------------


# ###### Number of PEs ######################################################
mecCalcAppPoolElementsDistribution <- function(currentBlock, totalBlocks,
                                               variable, gamma, lambda)
{
   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (variable < 0.0)) {
      stop("mecCalcAppPoolElementsDistribution: Check parameters!")
   }
   if(totalBlocks != 3) {
      stop("mecCalcAppPoolElementsDistribution: Invalid setting of totalBlocks!")
   }

   if(currentBlock == 1) {   # Local: As many PEs as PUs!


#     !!!!!!!!!!!!!!!!!!!!!
#       return(c("mecCalcAppPoolElementsDistribution", 1, 1))
#     !!!!!!!!!!!!!!!!!!!!!

      return(c("mecCalcAppPoolElementsDistribution", as.numeric(scenarioNumberOfCalcAppPoolUsersVariable), as.numeric(scenarioNumberOfCalcAppPoolUsersVariable)))
   }
   else if(currentBlock == 2) {   # MEC
      return(c("mecCalcAppPoolElementsDistribution", as.numeric(mecNumberOfMECPoolElements), as.numeric(mecNumberOfMECPoolElements)))
   }
   else {   # Cloud
      return(c("mecCalcAppPoolElementsDistribution", as.numeric(variable), as.numeric(variable)))
   }
}


# ###### Number of PUs ######################################################
mecCalcAppPoolUsersDistribution <- function(currentBlock, totalBlocks,
                                            variable, gamma, lambda)
{
   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (variable < 0.0)) {
      stop("mecCalcAppPoolUsersDistribution: Check parameters!")
   }
   if(totalBlocks != 3) {
      stop("mecCalcAppPoolUsersDistribution: Invalid setting of totalBlocks!")
   }

   if(currentBlock == 1) {   # PUs are local-only!
      return(c("mecCalcAppPoolUsersDistribution", variable, variable))
   }
   else {   # No PU in MEC or Cloud!
      return(c("mecCalcAppPoolUsersDistribution", 0, 0))
   }
}


# ###### MEC WAN delay distribution #########################################
mecWANDelayDistribution <- function(currentNetwork, totalNetworks,
                                    componentType, currentComponent, totalComponents,
                                    variable, gamma, lambda)
{
   if((currentNetwork < 1) || (currentNetwork > totalNetworks) ||
      (currentComponent < 1) || (currentComponent > totalComponents) ||
      (variable < 0.0)) {
      stop("mecWANDelayDistribution: Check parameters!")
   }

   return(c("MECWANDelayDistribution",
            as.numeric(variable), as.numeric(variable)))
}


# ###### MEC LAN delay distribution #########################################
mecLANDelayDistribution <- function(currentNetwork, totalNetworks,
                                    componentType, currentComponent, totalComponents,
                                    variable, gamma, lambda)
{
   if((currentNetwork < 1) || (currentNetwork > totalNetworks) ||
      (currentComponent < 1) || (currentComponent > totalComponents) ||
      (variable < 0.0)) {
      stop("mecLANDelayDistribution: Check parameters!")
   }

   if(currentNetwork == 1) {
      return(c("MECLANDelayDistribution",
             as.numeric(variable), as.numeric(variable)))
   }
   else if(currentNetwork == 2) {
      r <- runif(1, as.numeric(scenarioNetworkMECMinDelay), as.numeric(scenarioNetworkMECMaxDelay))
      return(c("MECLANDelayDistribution", r, r))
   }
   else {
      r <- runif(1, as.numeric(scenarioNetworkCloudMinDelay), as.numeric(scenarioNetworkCloudMaxDelay))
      return(c("MECLANDelayDistribution", r, r))
   }
}


# ###### MEC capacity distribution ##########################################
mecCapacityDistribution <- function(currentBlock, totalBlocks,
                                    currentElement, totalElements,
                                    variable, gamma, lambda)
{
   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable <= 0.0)) {
      stop("mecCapacityDistribution: Check parameters!")
   }

   if(currentBlock == 1) {
      return(c("MECCapacityDistribution",
               as.numeric(mecLocalCapacityFactor) * as.numeric(variable),
               as.numeric(mecLocalCapacityFactor) * as.numeric(variable)))
   }
   else if(currentBlock == 2) {
      return(c("MECCapacityDistribution",
               as.numeric(mecMECCapacityFactor) * as.numeric(variable),
               as.numeric(mecMECCapacityFactor) * as.numeric(variable)))
   }
   else {
      return(c("MECCapacityDistribution",
               as.numeric(variable), as.numeric(variable)))
   }
}


# ###########################################################################

rspsim5DefaultConfiguration <- append(rspsim5DefaultConfiguration, list(
   list("mecNumberOfMECPoolElements",   4),

   list("mecLocalDelayVariable",         1.0),
   list("scenarioNetworkMECMinDelay",    5.0),
   list("scenarioNetworkMECMaxDelay",   15.0),
   list("scenarioNetworkCloudMinDelay", 30.0),
   list("scenarioNetworkCloudMaxDelay", 300.0),

   list("mecLocalCapacityFactor",       0.1),
   list("mecMECCapacityFactor",         0.5)
))

simulationConfigurations <- list(
   # Local + MEC + Cloud
   list("scenarioNumberOfLANs", 3),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "mecCalcAppPoolElementsDistribution"),
   list("scenarioNumberOfCalcAppPoolUsersDistribution",    "mecCalcAppPoolUsersDistribution"),

   list("scenarioNetworkWANDelayDistribution",             "mecWANDelayDistribution"),
   list("scenarioNetworkLANDelayDistribution",             "mecLANDelayDistribution"),
   list("calcAppPoolElementServiceCapacityDistribution",   "mecCapacityDistribution"),

   list("scenarioNumberOfCalcAppPoolElementsVariable",     10),
   list("mecNumberOfMECPoolElements",                      4),

   list("calcAppPoolElementSelectionPolicy",               "LeastUsedDegradation", "PriorityLeastUsed", "PriorityLeastUsedDegradation", "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"),
   list("calcAppPoolElementSelectionPolicyLoadDPF",        0.0001),
   list("calcAppPoolElementSelectionPolicyWeightDPF",      0.0001),

   list("calcAppPoolElementServiceCapacityVariable",       1000000),
   list("calcAppPoolElementServiceMinCapacityPerJob",      200000),
   list("mecLocalCapacityFactor",                          0.2),   # !!!!!!!
   list("mecMECCapacityFactor",                            1.0),   # !!!!!!!

   list("scenarioNumberOfCalcAppPoolUsersVariable",        1, 5, 25, 50, 100),
   list("calcAppPoolUserServiceJobSizeVariable",           1e4, 5e4, 1e5, 1.5e5, 2e5, 2.5e5, 3e5, 3.5e5, 4e5, 4.5e5, 5e5),
   list("calcAppPoolUserServiceJobIntervalVariable",       10),

   list("scenarioNetworkLANDelayVariable",                   1.0),   # Local
   list("scenarioNetworkMECMinDelay",                        5.0),   # MEC (lower bound)
   list("scenarioNetworkMECMaxDelay",                       15.0),   # MEC (lower bound)
   list("scenarioNetworkCloudMinDelay",                     30.0),   # Cloud (lower bound)
   list("scenarioNetworkCloudMaxDelay",                    300.0),   # Cloud (upper bound)

   list("SPECIAL0", "gammaScenario.lan[0].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 1.00"),   # Local: 100%
   list("SPECIAL1", "gammaScenario.lan[1].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 0.10"),   # MEC: 10%
   list("SPECIAL2", "gammaScenario.lan[2].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 0.20")    # Cloud: 20%
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
