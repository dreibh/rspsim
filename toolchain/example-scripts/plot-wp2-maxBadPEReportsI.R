# ###########################################################################
# Name:        wp2-maxBadPEReportsI
# Description: Reducing monitoring overhead with MaxBadPEReports
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "wp2-maxBadPEReportsI"
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
        "Provider's Perspective", NA, seq(29, 39, 1), list(0.5,0),
        "registrarMaxBadPEReports", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", NA, seq(5, 50, 5), list(0.9,1),
        "registrarMaxBadPEReports", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ProcessingSpeed.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "registrarMaxBadPEReports", "controller.SystemAverageProcessingSpeed",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-StartupDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "registrarMaxBadPEReports", "controller.SystemAverageStartupDelay",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-QueuingDelay.pdf"),
        "User's Perspective", NA, NA, list(0,1),
        "registrarMaxBadPEReports", "controller.SystemAverageQueuingDelay",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout"),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-ASAPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "registrarMaxBadPEReports", "controller.TransportNodeGlobalASAPPackets",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout"),
   list(simulationDirectory, paste(sep="", simulationDirectory, "-ENRPPackets.pdf"),
        "Provider's Perspective", NA, NA, list(1,1),
        "registrarMaxBadPEReports", "controller.TransportNodeGlobalENRPPackets",
        "calcAppPoolElementSelectionPolicy", "asapEndpointKeepAliveInterval", "asapEndpointKeepAliveTimeout")
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
