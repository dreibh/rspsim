# ###########################################################################
# Name:        tos-pr-failureIII
# Description: Takeover Suggestion option:
#              PR #1 is always available, PRs #2 to #5 have problems
#                 => Distribution of PEs among PRs using Takeover Suggestion.
#                    Otherwise: PR #1 will take them all.
#              Parameters corresponding to lab setup (50%, delay, PU:PE ratio 3)
# ###########################################################################

source("simulate-version14.R")

# ====== Simulation Settings ================================================
simulationDirectory <- "tos-pr-failureIII"
simulationRuns <- 36
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
   list("scenarioNumberOfRegistrarsVariable", 5),
   list("scenarioNumberOfCalcAppPoolElementsVariable", 100),
   list("scenarioNetworkLANDelayVariable", 25),
   list("scenarioNetworkLANDelayDistribution", "uniformDelayDistribution"),   # !!!

   list("asapMaxRequestRetransmit", 1),
   list("asapRequestTimeout", 5),
   list("asapEndpointKeepAliveInterval", 30),
   list("asapEndpointKeepAliveTimeout", 5),
   list("asapUseTakeoverSuggestion", "false", "true"),

   list("registrarRandomizeMaxHandleResolutionItems", "false"),
   list("registrarMaxHandleResolutionItems", 1),
   list("registrarMaxIncrement", 1),

   # Verteilung zuerst auf alle PRs!
   # ==== list("calcAppPoolElementStaticRegistrarList", "1"),   ====

   list("registrarComponentUptimeDistribution", "timeExponentialRandomizedDistribution"),
   list("registrarComponentDowntimeDistribution", "timeExponentialRandomizedDistribution"),
   list("registrarComponentUptimeVariable", 200, 300, 400, 500, 625, 775, 1000),
   list("registrarComponentDowntimeVariable", 100),
   list("SPECIAL0", "gammaScenario.lan[*].registrarArray[0].registrarProcess.componentUptime = 100d"),

   list("calcAppPoolElementSelectionPolicy", "LeastUsed", "RoundRobin", "Random"),
   list("calcAppPoolUserServiceJobSizeVariable", 1e7)
)

# ###########################################################################

createSimulation(simulationDirectory, simulationConfigurations, rspsim5DefaultConfiguration)
