# ###########################################################################
# Name:        mec2-baseline.R
# Description: Baseline scenario with 14 PMC PEs, without EC
# ###########################################################################

source("simulate-version14.R")


# ====== Simulation Settings ================================================
simulationDirectory <- "mec2-baseline"
simulationRuns <- 3
simulationDuration <- 7*24*60 - 21   # 1 week - 21 min
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
      return(c("mecCalcAppPoolElementsDistribution", 0,0 ))   # !!! No PEs on UE !!!
      # return(c("mecCalcAppPoolElementsDistribution",  as.numeric(scenarioNumberOfCalcAppPoolUsersVariable), as.numeric(scenarioNumberOfCalcAppPoolUsersVariable)))
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


# ###### Workload distribution ##############################################
reqdistfromfileJobIntervalDistribution <- function(currentBlock, totalBlocks,
                                                   currentElement, totalElements,
                                                   variable, gamma, lambda)
{
   variable <- as.numeric(variable)
   gamma    <- as.numeric(gamma)
   lambda   <- as.numeric(lambda)

   if((currentBlock < 1) || (currentBlock > totalBlocks) ||
      (currentElement < 1) || (currentElement > totalElements) ||
      (variable != 0.0)) {
      stop("customWorkloadDistribution: Check parameters!")
   }

   return(c("Special",
            sprintf("reqdistfromfile(\"7day_task_req.csv\") * %d", as.numeric(scenarioNumberOfCalcAppPoolUsersVariable)),
            NA))
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
   list("mecMECCapacityFactor",         0.5),

   list("totalRequestsPerMinute",       7.0)   # <<-- Requests/min
))

simulationConfigurations <- list(
   # Local + MEC + Cloud
   list("scenarioNumberOfLANs", 3),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "mecCalcAppPoolElementsDistribution"),
   list("scenarioNumberOfCalcAppPoolUsersDistribution",    "mecCalcAppPoolUsersDistribution"),

   list("scenarioNetworkWANDelayDistribution",             "mecWANDelayDistribution"),
   list("scenarioNetworkLANDelayDistribution",             "mecLANDelayDistribution"),
   list("calcAppPoolElementServiceCapacityDistribution",   "mecCapacityDistribution"),

   list("mecNumberOfMECPoolElements",                       0),   # MEC
   list("scenarioNumberOfCalcAppPoolElementsVariable",     14),   # Cloud

   list("calcAppPoolElementSelectionPolicy",               
     "Random", "RoundRobin",
     "LeastUsed",
     "PriorityLeastUsed", "PriorityLeastUsedDegradation"
     # "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"
   ),
   list("calcAppPoolElementSelectionPolicyLoadDPF",        0.0001),
   list("calcAppPoolElementSelectionPolicyWeightDPF",      0.0001),

   list("calcAppPoolElementServiceCapacityVariable",       300*60),
   list("calcAppPoolElementServiceMinCapacityPerJob",      300*60 / 2),
   list("mecLocalCapacityFactor",                          0.05),   # 0.05*300 = 15
   list("mecMECCapacityFactor",                            0.5),    # 0.5*300  = 150

   list("scenarioNumberOfCalcAppPoolUsersVariable",        10, 20, 30, 40, 50, 60),

   list("calcAppPoolUserServiceJobSizeVariable",           450*300*60),   # 6*75=450
   list("calcAppPoolUserServiceJobSizeDistribution",       "workloadUniformRandomizedDistribution"), 
   list("calcAppPoolUserServiceJobSizeGamma",              4),   # --> uniform(0.5*jobSize,1.5*jobSize)


   # list("calcAppPoolUserServiceJobIntervalVariable",       2571.429),   # 62.5% utilisation for 50 PUs
   # list("calcAppPoolUserServiceJobIntervalDistribution",   "workloadUniformRandomizedDistribution"),
   # list("calcAppPoolUserServiceJobIntervalGamma",          4),   # --> uniform(0.25*jobSize,4*IntervalSize)

   list("calcAppPoolUserServiceJobIntervalVariable",       0.0),   # Ca. 62.5% utilisation for 50 PUs, distribution from file 7day_task_req.csv!
   list("calcAppPoolUserServiceJobIntervalDistribution",   "reqdistfromfileJobIntervalDistribution"),   # <<-- customised, see function above!


   list("scenarioNetworkLANDelayVariable",                   1.0),   # Local
   list("scenarioNetworkMECMinDelay",                        5.0),   # MEC (lower bound)
   list("scenarioNetworkMECMaxDelay",                       15.0),   # MEC (lower bound)
   list("scenarioNetworkCloudMinDelay",                     30.0),   # Cloud (lower bound)
   list("scenarioNetworkCloudMaxDelay",                    300.0),   # Cloud (upper bound)

   list("SPECIAL0", "gammaScenario.lan[0].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 1.00"),   # Local: 100%
   list("SPECIAL1", "gammaScenario.lan[1].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 0.10"),   # MEC: 10%
   list("SPECIAL2", "gammaScenario.lan[2].calcAppPoolElementArray[*].calcAppServer.selectionPolicyLoadDegradation = 0.20")    # Cloud: 20%
)

simCreatorMiscFiles <- list(c("7day_task_req.csv", "."))   # <<-- The requests CSV file

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
