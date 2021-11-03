# ###########################################################################
# Name:        wp1-pr-selectionI
# Description: Varying the number of registrars
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "wp1-pr-selectionI"
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
   list("puToPERatio", 25),
   list("scenarioNumberOfRegistrarsVariable", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxHandleResolutionItems", 999),
   list("registrarMaxIncrement", 1, 2, 4, 5, 99),
   list("calcAppPoolElementSelectionPolicy", "RoundRobin"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
