# ###########################################################################
# Name:        attack-registrations-countermeasureIII
# Description: Countermeasure for registration attacks:
#              only a single PE per user (by fixed user-specific PE ID)
#              Varying the number of attackers
#              Parameters corresponding to lab setup (50%, delay, PU:PE ratio 3)
# ###########################################################################

source("simulate-version14.R")

# ------ Plotter Settings ---------------------------------------------------
simulationDirectory  <- "attack-registrations-countermeasureIII"
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
filter <- TRUE   # "(data1$registrarMaxBadPEReports == 3)"

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
        "Provider's Perspective", list(seq(0,10,1)), list(seq(10,60,10)), list(1,0),
        "scenarioNumberOfAttackersVariable", "controller.SystemAverageUtilization",
        "calcAppPoolElementSelectionPolicy", "attackerAttackInterval", "",
         "", "", "", filter),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-HandlingSpeed.pdf"),
        "User's Perspective", list(seq(0,10,1)), list(seq(0,90,10)), list(1,0),
        "scenarioNumberOfAttackersVariable", "controller.SystemAverageHandlingSpeed",
        "calcAppPoolElementSelectionPolicy", "attackerAttackInterval", "",
         "", "", "", filter),

   list(simulationDirectory, paste(sep="", simulationDirectory, "-IgnoredAppPackets.pdf"),
        "User's Perspective", list(seq(0,10,1)), NA, list(1,1),
        "scenarioNumberOfAttackersVariable", "controller.GlobalAttackerIgnoredApplicationMessages",
        "calcAppPoolElementSelectionPolicy", "attackerAttackInterval", "",
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
