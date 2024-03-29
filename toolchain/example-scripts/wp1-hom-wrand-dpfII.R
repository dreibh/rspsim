# ###########################################################################
# Name:        wp1-hom-wrand-dpfII
# Description: WRAND-DPF policy proof of concept: request size variation
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-hom-wrand-dpfII"
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
   list("puToPERatio", 3, 10),

   list("scenarioNumberOfLANs", 3),
   list("scenarioNetworkWANDelayVariable", 0, 175, 300),
   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 50),
   list("asapEndpointKeepAliveTimeout", 50),
   list("calcAppProtocolServiceJobKeepAliveInterval", 50),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 50),

   list("calcAppPoolElementSelectionPolicy", "WeightedRandomDPF"),
   list("calcAppPoolElementSelectionPolicyWeightDPF", 0.0, 0.008),
   list("calcAppPoolUserServiceJobSizeVariable", 1e5, 2.5e5, 5e5, 1e6, 1.5e6, 2e6, 3.25e6, 5e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
