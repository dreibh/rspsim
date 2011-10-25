# ###########################################################################
# Name:        wp1-het-linearI
# Description: 1/3 fast servers capacity distribution
# Revision:    $Id$
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "wp1-het-linearI"
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

filter <- "(data1$calcAppPoolElementSelectionPolicy != \"WeightedRandom\") | (data1$puToPERatio == 10)"

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

   list(simulationDirectory, paste(sep="", simulationDirectory, "-Utilization.pdf"),
        "Provider's Perspective", NA, seq(30, 90, 10), list(0,0),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
        "", "", "", filter),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, seq(0, 80, 10), list(1,0.5),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
        "", "", "", filter),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ProcessingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.SystemAverageProcessingSpeed",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
        "", "", "", filter),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-StartupDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.SystemAverageStartupDelay",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
        "", "", "", filter),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-QueuingDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.SystemAverageQueuingDelay",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
        "", "", "", filter),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ASAPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.TransportNodeGlobalASAPPackets",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
        "", "", "", filter),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-ENRPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "calcAppPoolElementServiceCapacityGamma.Linear", "controller.TransportNodeGlobalENRPPackets",
        "calcAppPoolElementSelectionPolicy", "puToPERatio", "jsToSC",
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
