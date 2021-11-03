# ###########################################################################
# Name:        wp1-disaster-wrand-dpfI
# Description: Disaster scenario with WRAND-DPF
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-disaster-wrand-dpfI"
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
   list("puToPERatio", 10),

   list("scenarioNumberOfLANs", 3),
   list("scenarioNumberOfCalcAppPoolElementsDistribution", "localizedDisasterDistribution"),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 12),
   list("scenarioNumberOfCalcAppPoolElementsGamma", 1),
   list("scenarioNumberOfCalcAppPoolElementsLambda", 1.00, 0.75, 0.50, 0.25),

   list("scenarioNetworkWANDelayVariable", 150),
   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 50),
   list("asapEndpointKeepAliveTimeout", 50),
   list("calcAppProtocolServiceJobKeepAliveInterval", 50),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 50),

   list("calcAppPoolElementSelectionPolicy", "WeightedRandomDPF"),
   list("calcAppPoolElementSelectionPolicyWeightDPF",
      0.000, 0.001, 0.0015,
      0.002, 0.0025, 0.003, 0.0035, 0.004, 0.0045,
      0.005, 0.0055, 0.006, 0.007,
      0.008),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
