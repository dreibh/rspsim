# ###########################################################################
# Name:        example1
# Description:
# ###########################################################################

source("simulate-version14.R")


# ====== Simulation Settings ================================================
simulationDirectory <- "example1"
simulationRuns <- 3
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
   list("puToPERatio", 1, 2, 3, 5, 7, 10, 15, 20),

   list("scenarioNumberOfCalcAppPoolElementsVariable", 10),

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "Random", "RoundRobin"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e6, 1e8)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
