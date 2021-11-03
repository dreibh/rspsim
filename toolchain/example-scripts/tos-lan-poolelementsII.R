# ###########################################################################
# Name:        tos-lan-poolelementsII
# Description: Varying the number of PEs (LAN setup)
#              Parameters corresponding to lab setup (50%, delay, PU:PE ratio 3)
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "tos-lan-poolelementsII"
simulationRuns <- 24
simulationDuration <- 120
simulationStoreVectors <- FALSE
simulationExecuteMake <- TRUE
simulationScriptOutputVerbosity <- 3
simulationSummaryCompressionLevel <- 9
simulationSummarySkipList <- c("lan.calcApp", "lan.switch", "lan.attacker", "lan.registrarArray.transport")
# -------------------------------------
source("computation-pool.R")
# -------------------------------------

# ###########################################################################

simulationConfigurations <- list(
   list("puToPERatio", 3),
   list("targetSystemUtilization", 0.50),
   list("scenarioNumberOfRegistrarsVariable", 1, 2, 3, 4, 5),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 25, 50, 100),
   list("scenarioNetworkLANDelayVariable", 0, 10, 150),
   list("scenarioNetworkLANDelayDistribution", "uniformDelayDistribution"),   # !!!

   list("asapUseTakeoverSuggestion", "false"),

   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxHandleResolutionItems", 1),
   list("registrarMaxIncrement", 1),

   # --- list("calcAppPoolElementStaticRegistrarList", "1"),   # !!!

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "RoundRobin", "Random"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
