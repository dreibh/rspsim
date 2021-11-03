# ###########################################################################
# Name:        attack-registrationsIV
# Description: What happens in case of registration attacks?
#              (LoadDeg is 25%, Attacker 0%, MinCapPerReq=250000)
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "attack-registrationsIV"
plotColorMode        <- cmColor
plotHideLegend       <- FALSE
plotLegendSizeFactor <- 0.8
plotOwnOutput        <- FALSE
plotFontFamily       <- "Helvetica"
plotFontPointsize    <- 22
plotWidth            <- 10
plotHeight           <- 10
plotConfidence       <- 0.95

# ###########################################################################

# ------ Plots --------------------------------------------------------------
filter <- TRUE # "data1$registrarMaxBadPEReports == 10"

plotConfigurations <- list(
   # ------ Format example --------------------------------------------------
   # list(simulationDirectory, "output.pdf",
   #      "Plot Title",
   #      list(xAxisTicks) or NA, list(yAxisTicks) or NA, list(legendPos) or NA,
   #      "x-Axis Variable", "y-Axis Variable",
   #      "z-Axis Variable", "v-Axis Variable", "w-Axis Variable",
   #      "a-Axis Variable", "b-Axis Variable", "p-Axis Variable")
   # ------------------------------------------------------------------------

  list(simulationDirectory, paste(sep="", simulationDirectory, "-Utilization.pdf"),
        "Provider's Perspective", NA, NA, list(1,0),
        "attackerAttackInterval", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "registrarMaxBadPEReports", "attackRandomizeIdentifier",
        "", "", "", filter),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, list(seq(0,70,10)), list(1,0),
        "attackerAttackInterval", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "registrarMaxBadPEReports", "attackRandomizeIdentifier",
        "", "", "", filter),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-IgnoredAppPackets.pdf"),
        "User's Perspective", NA, list(seq(0, 75000, 25000)), list(1,1),
        "attackerAttackInterval", "controller.GlobalAttackerIgnoredApplicationMessages",
        "calcAppPoolElementSelectionPolicy", "registrarMaxBadPEReports", "attackRandomizeIdentifier",
        "", "", "", filter)
)


# ------ Variable templates -------------------------------------------------
plotVariables <- append(list(
   # ------ Format example --------------------------------------------------
   # list("Variable",
   #         "Unit[x]{v]"
   #          "100.0 * data1$x / data1$y", <- Manipulator expression:
   #                                           "data" is the data table
   #                                        NA here means: use data1$Variable.
   #          "myColor",
   #          list("InputFile1", "InputFile2", ...))
   #             (simulationDirectory/Results/....data.tar.bz2 is added!)
   # ------------------------------------------------------------------------

   # list("controller.SystemAverageUtilization",
   #         "Average Utilization[%]",
   #         "100.0 * data1$controller.SystemAverageUtilization",
   #         "blue4",
   #         list("controller-SystemAverageUtilization"))

), rspsim5PlotVariables)

# ###########################################################################

createPlots(simulationDirectory, plotConfigurations)
