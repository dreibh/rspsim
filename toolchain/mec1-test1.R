# ###########################################################################
# Name:        example1
# Description:
# ###########################################################################

source("simulate-version14.R")


# ====== Simulation Settings ================================================
simulationDirectory <- "mec1-test1"
simulationRuns <- 1
simulationDuration <- 120
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 8
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
      return(c("mecCalcAppPoolUsersDistribution", 1, 1))
   }
   else {   # No PU in MEC or Cloud!
      return(c("mecCalcAppPoolUsersDistribution", 0, 0))
   }
}


# ###### Identity distribution ##############################################
mecDelayDistribution <- function(currentNetwork, totalNetworks,
                                 componentType, currentComponent, totalComponents,
                                 variable, gamma, lambda)
{
   if((currentNetwork < 1) || (currentNetwork > totalNetworks) ||
      (currentComponent < 1) || (currentComponent > totalComponents) ||
      (variable < 0.0)) {
      stop("mecDelayDistribution: Check parameters!")
   }

   if(currentNetwork == 1) {
      return(c("MECDelayDistribution",
             as.numeric(mecMECDelayVariable),
             as.numeric(mecMECDelayVariable)))
   }
   else {
      return(c("MECDelayDistribution",
               as.numeric(variable),
               as.numeric(variable)))
   }
}


# ###### Identity distribution ##############################################
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
               as.numeric(variable),
               as.numeric(variable)))
   }
}


# ###########################################################################

rspsim5DefaultConfiguration <- append(rspsim5DefaultConfiguration, list(
   list("mecNumberOfMECPoolElements", 4),

   list("mecLocalDelayVariable", 1.0),
   list("mecMECDelayVariable", 10.0),

   list("mecLocalCapacityFactor", 0.1),
   list("mecMECCapacityFactor", 0.5)
))

simulationConfigurations <- list(
   # Local + MEC + Cloud
   list("scenarioNumberOfLANs", 3),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "mecCalcAppPoolElementsDistribution"),
   list("scenarioNumberOfCalcAppPoolUsersDistribution",    "mecCalcAppPoolUsersDistribution"),

   list("scenarioNetworkWANDelayDistribution",             "mecDelayDistribution"),
   list("calcAppPoolElementServiceCapacityDistribution",   "mecCapacityDistribution"),

   list("scenarioNumberOfCalcAppPoolElementsVariable", 10),
   list("mecNumberOfMECPoolElements", 4),

   list("calcAppPoolElementSelectionPolicy", "PriorityLeastUsed"),
   # , "Random", "RoundRobin", "LeastUsed", ),
   list("calcAppPoolElementServiceCapacityVariable", 1000000),

   list("scenarioNumberOfCalcAppPoolUsersVariable"),
   list("puToPERatio", 0.1, 0.5, 1, 2, 5, 10),   # !!! Based on scenarioNumberOfCalcAppPoolElementsVariable ONLY! !!!

   list("calcAppPoolUserServiceJobSizeVariable",     1000000),
#    1e6, 1e8)
   list("calcAppPoolUserServiceJobIntervalVariable", 10),

   list("scenarioNetworkLANDelayVariable", 1.0),     # LAN (i.e. also: Local)
   list("mecMECDelayVariable", 10.0),                # MEC
   list("scenarioNetworkWANDelayVariable", 200.0),   # Cloud

   list("mecLocalCapacityFactor", 0.1),
   list("mecMECCapacityFactor",   0.5),

   list("SPECIAL0", "gammaScenario.lan[0].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 1.00"),   # Local: 100%
   list("SPECIAL1", "gammaScenario.lan[1].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 0.10"),   # MEC: 10%
   list("SPECIAL2", "gammaScenario.lan[2].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 0.50")    # Cloud: 50%
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
