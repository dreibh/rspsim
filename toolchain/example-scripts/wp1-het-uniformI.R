# ###########################################################################
# Name:        wp1-het-uniformI
# Description: Uniform capacity distribution
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-het-uniformI"
simulationRuns <- 24
simulationDuration <- 120
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
   list("puToPERatio", 1, 3, 10),

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "WeightedRandom"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6, 1e8),

   list("calcAppPoolElementServiceCapacityDistribution", "workloadUniformRandomizedDistribution"),
   list("calcAppPoolElementServiceCapacityGamma", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0),
   list("calcAppPoolElementServiceCapacityLambda", 0.0)

)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
