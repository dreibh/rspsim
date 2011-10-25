# ###########################################################################
# Name:        plot-lud-puToPERatioI.R
# Description:
# Revision:    $Id$
# ###########################################################################

source("simulate-version11.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "lud-puToPERatioI"
plotColorMode        <- cmColor
plotHideLegend       <- FALSE
plotLegendSizeFactor <- 0.8
plotOwnOutput        <- FALSE
plotFontFamily       <- "Helvetica"
plotFontPointsize    <- 18
plotWidth            <- 10
plotHeight           <- 10
plotConfidence       <- 0.95

# ###########################################################################

# ------ Plots --------------------------------------------------------------
plotConfigurations <- list(
   # ------ Format example --------------------------------------------------
   # list(simulationDirectory, "output.pdf",
   #      "Plot Title",
   #      list(xAxisTicks) or NA, list(yAxisTicks) or NA, list(legendPos) or NA,
   #      "x-Axis Variable", "y-Axis Variable",
   #      "z-Axis Variable", "v-Axis Variable", "w-Axis Variable",
   #      "a-Axis Variable", "b-Axis Variable", "p-Axis Variable")
   # ------------------------------------------------------------------------

   # ------ Request Interval/PU:PE Ratio -> Request Size --------------------
   list(simulationDirectory, paste(sep="", simulationDirectory, "-Utilization.pdf"),
        "Provider's Perspective", NA, NA, list(1,0),
        "puToPERatio", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "jsToSC", ""),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "puToPERatio", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "jsToSC", ""),

#    list(simulationDirectory, paste(sep="", simulationDirectory, "-ASAPPackets.pdf"),
#         "Provider's Perspective", NA, NA, list(1,1),
#         "calcAppPoolElementSelectionPolicyLoadDegradation", "controller.TransportNodeGlobalASAPPackets",
#         "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC"),
#    list(simulationDirectory, paste(sep="", simulationDirectory, "-ENRPPackets.pdf"),
#         "Provider's Perspective", NA, NA, list(1,1),
#         "calcAppPoolElementSelectionPolicyLoadDegradation", "controller.TransportNodeGlobalENRPPackets",
#         "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC")
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
