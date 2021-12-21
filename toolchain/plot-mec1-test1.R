#!/usr/bin/Rscript
#
# SELF — Self-contained User Data Preserving Framework
# Copyright (C) 2021 by Thomas Dreibholz
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Contact: dreibh@simula.no

library("anytime")
library("data.table")
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")


plotColours <- c("steelblue4", "steelblue1",
                 "salmon4",    "salmon1",
                 "seagreen4",  "seagreen1",
                 "goldenrod3", "goldenrod1",
                 "gray40",     "gray10",
                 "cyan4",      "cyan1",
                 "yellow4",    "yellow1")


# ###### Read output file ###################################################
readResults <- function(directory)
{
   dataName <- paste(sep="/", directory, "controller-CalcAppPEGlobalUsedCapacity.data.bz2")
   print(paste(sep="", "Trying to read ", dataName, " ..."))
   data <- read.table(pipe(paste(sep="", "bzcat ", dataName)))
   # names(dataData)[names(dataData) == "data.flow"] <- "FlowID"
   # names(dataData)[names(dataData) == "data.flow.ReceivedBitRate"] <- "flow.ReceivedBitRate"
#
#    data <- rbind(dataData, passiveData)

   # Convert time stamp with anytime() (time is in UTC!):
   # data$TimeStamp <- anytime(data$TimeStamp, tz="UTC")

   return(data)
}


# ###### Plot results #######################################################
plotPEUtilisation <- function(name)
{
   # ====== Plot as PDF file ================================================
   calcAppPETotalUsedCapacity <- data.table(loadResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalUsedCapacity.data.bz2")))
   calcAppPETotalWastedCapacity <- data.table(loadResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalWastedCapacity.data.bz2")))
   print(sort(colnames(calcAppPETotalUsedCapacity)))

   cairo_pdf(paste(sep="", name, "-CalcAppPETotalUsedCapacity.pdf"),
            width=12, height=8, family="Helvetica", pointsize=22)

   title <- ""

   # ====== Calculate utilisation ===========================================
   utilisation <- data.table(utilisation = 100.0 * calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity /
                               (calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity + calcAppPETotalWastedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalWastedCapacity))
   calcAppPETotalUsedCapacity <- cbind(calcAppPETotalUsedCapacity, utilisation)

   # ====== Use factors for LAN and LAN.CalcAppPoolElementArray =============
   calcAppPETotalUsedCapacity$lan <- factor(calcAppPETotalUsedCapacity$lan)
   calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray <- factor(calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray)


   plotColours <- c(
      "red1",  "green4", "green1",  "blue1", "blue4", "purple1", "purple4"
   )


   # ====== Use dplyr to summarise results ==================================
   summarised <- calcAppPETotalUsedCapacity %>%
                    group_by(calcAppPoolElementSelectionPolicy,scenarioNumberOfCalcAppPoolUsersVariable,lan,lan.calcAppPoolElementArray) %>%
                    summarise(#.groups = "keep",   # Keep the grouping as is. Otherwise, it would drop the last one!
                              MeanCalcAppPEUtilisation = mean(utilisation),
                              MinCalcAppPEUtilisation  = min(utilisation),
                              MaxCalcAppPEUtilisation  = max(utilisation),
                              Q10CalcAppPEUtilisation  = quantile(utilisation, 0.10),
                              Q90CalcAppPEUtilisation  = quantile(utilisation, 0.90))
   print(summarised)


   # ====== Create plots ====================================================
   p <- ggplot(summarised,
               aes(x = scenarioNumberOfCalcAppPoolUsersVariable,
                   y = MeanCalcAppPEUtilisation,
                   color = lan.calcAppPoolElementArray)
            ) +
         theme(title             = element_text(size=32),
               plot.title        = element_text(size=20, hjust = 0.5, face="bold"),
               axis.title        = element_text(size=20, face="bold"),
               strip.text        = element_text(size=18, face="bold"),
               axis.text.x       = element_text(size=16, angle=90, face="bold", colour="black"),
               axis.text.y       = element_text(size=16, angle=90, hjust=0.5, colour="black"),
               legend.title      = element_blank(),
               legend.text       = element_text(size=18, face="bold"),
#                legend.background = element_rect(colour = bgColor,  fill = lgColor, size=1),
#                panel.background  = element_rect(fill = paste(sep="", "#", colorCU), color=bgColor, size=2),
               panel.grid.major  = element_line(size=0.5,  linetype="solid", color="lightgray"),
               panel.grid.minor  = element_line(size=0.25, linetype="solid", color="lightgray")
               ) +
         labs(title = title,
               x     = "Number of Pool User Instances on the UEs [1]",
               y     = "Average Utilisation [%]") +
         facet_grid(lan ~ calcAppPoolElementSelectionPolicy) +
#          facet_wrap(~calcAppPoolElementSelectionPolicy) +
         geom_line(aes(color = lan.calcAppPoolElementArray), size = 2) +
         geom_errorbar(aes(ymin = MinCalcAppPEUtilisation, ymax = MaxCalcAppPEUtilisation, color=lan),
                        size=1.5, width=.5) +
         geom_errorbar(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=lan),
                        size=0.5, width=.25)
         geom_ribbon(aes(ymin = Q10CalcAppPEUtilisation, ymax = Q90CalcAppPEUtilisation, color=lan),
                     size=0.01, linetype=2, alpha=0.1)
         # xlim(0, 120) +
#          scale_color_manual(values = plotColours)   # Line colour

   print(p)

   dev.off()
   return(summarised)
}


# ###### Main program #######################################################

data <- plotPEUtilisation("mec1-test1/Results")
