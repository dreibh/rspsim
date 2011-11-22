# ###########################################################################
# Name:        wp1-hom-lu-dpfI
# Description: LU-DPF policy proof of concept: WAN delay variation
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-hom-lu-dpfI"
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
   list("puToPERatio", 1, 3, 10),

   list("scenarioNumberOfLANs", 3),
   list("scenarioNetworkWANDelayVariable", 0, 50, 100, 175, 250, 375, 500),
   list("scenarioNetworkLANDelayVariable", 10),

   list("asapEndpointKeepAliveInterval", 50),
   list("asapEndpointKeepAliveTimeout", 50),
   list("calcAppProtocolServiceJobKeepAliveInterval", 50),
   list("calcAppProtocolServiceJobKeepAliveTimeout", 50),

   list("calcAppPoolElementSelectionPolicy", "LeastUsedDPF"),
   list("calcAppPoolElementSelectionPolicyLoadDPF", 0.0, 0.00001),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
