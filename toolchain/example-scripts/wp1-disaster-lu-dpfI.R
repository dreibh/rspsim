# ###########################################################################
# Name:        wp1-disaster-lu-dpfI
# Description: Disaster scenario with LU-DPF
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-disaster-lu-dpfI"
simulationRuns <- 24
simulationDuration <- 60
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 3
simulationSummaryCompressionLevel <- 9
simulationSummarySkipList <- c("lan.")
# -------------------------------------
source("computation-pool.R")
# -------------------------------------

# ###########################################################################

simulationConfigurations <- list(
   list("targetSystemUtilization", 0.60),
   list("puToPERatio", 3),

   list("scenarioNumberOfLANs", 3),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "localizedDisasterDistribution"),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 12),
   list("scenarioNumberOfCalcAppPoolElementsGamma", 1),
   list("scenarioNumberOfCalcAppPoolElementsLambda", 1.00, 0.25, 0.75, 0.50),

   list("scenarioNetworkWANDelayVariable", 150),
   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 50),
   list("asapEndpointKeepAliveTimeout", 50),
   list("calcAppProtocolServiceJobKeepAliveInterval", 50),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 50),

   list("calcAppPoolElementSelectionPolicy", "LeastUsedDPF"),
   list("calcAppPoolElementSelectionPolicyLoadDPF",
        0.00000,
        0.00001, 0.00002, 0.00003, 0.00005,
        0.00010, 0.00015,
        0.00020),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
