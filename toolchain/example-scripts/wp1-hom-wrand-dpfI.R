# ###########################################################################
# Name:        wp1-hom-wrand-dpfI
# Description: WRAND-DPF policy proof of concept: WAN delay variation
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-hom-wrand-dpfI"
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
   list("scenarioNetworkWANDelayVariable", 0, 50, 100, 175, 250, 375, 500, 750, 1000),
   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 50),
   list("asapEndpointKeepAliveTimeout", 50),
   list("calcAppProtocolServiceJobKeepAliveInterval", 50),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 50),

   list("calcAppPoolElementSelectionPolicy", "WeightedRandomDPF"),
   list("calcAppPoolElementSelectionPolicyWeightDPF", 0.0, 0.002, 0.005, 0.008),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
