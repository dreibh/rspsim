#!/usr/bin/Rscript
#
# MEC/PMC performance analysis of DPF policies
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
library("plyr",  warn.conflicts = FALSE)
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")
library("xtable")


plotColours <- c("steelblue4", "steelblue1",
                 "salmon4",    "salmon1",
                 "seagreen4",  "seagreen1",
                 "goldenrod3", "goldenrod1",
                 "gray40",     "gray10",
                 "cyan4",      "cyan1",
                 "yellow4",    "yellow1")


# ###### Read results file ###################################################
readResults <- function(name)
{
   cat(sep="", "Reading ", name, " ...\n")
   data <- fread(name, sep="\t", quote='"')
   colnames(data) <- gsub("-", ".", colnames(data))

   # print(sort(colnames(data)))
   return(data)
}


# ###### Get policy types ###################################################
getPolicyType <- function(policies)
{
   return(as.factor(mapvalues(as.vector(policies),
          from = c("Random", "RoundRobin",
                   "LeastUsed", "LeastUsedDegradation", "PriorityLeastUsed", "PriorityLeastUsedDegradation", "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"),
          to   = c("Non-Adaptive", "Non-Adaptive",
                   "Adaptive", "Adaptive",              "Adaptive",          "Adaptive",                     "Adaptive with DPF",    "Adaptive with DPF"),
                   warn_missing = FALSE)))
}


# ###### Get policy types ###################################################
getPolicyType2 <- function(policies)
{
   return(as.factor(mapvalues(as.vector(policies),
          from = c("Random", "RoundRobin",
                   "LeastUsed", "LeastUsedDegradation", "PriorityLeastUsed", "PriorityLeastUsedDegradation", "PriorityLeastUsedDPF", "PriorityLeastUsedDegradationDPF"),
          to   = c("Non-Adaptive", "Non-Adaptive",
                   "Adaptive", "Adaptive",              "Adaptive",          "Adaptive",                     "Adaptive/DPF",    "Adaptive/DPF"),
                   warn_missing = FALSE)))
}


# ###### Get policy abbreviation ############################################
getPolicyAbbreviations <- function(policies)
{
   policies <-
      recode_factor(as.factor(policies),
                    "Random"                          = "RAND",
                    "RoundRobin"                      = "RR",
                    "LeastUsed"                       = "LU",
                    "LeastUsedDegradation"            = "LUD",
                    "PriorityLeastUsed"               = "PLU",
                    "PriorityLeastUsedDegradation"    = "PLUD",
                    "PriorityLeastUsedDPF"            = "PLU-DPF",
                    "PriorityLeastUsedDegradationDPF" = "PLUD-DPF"

#                     "Random"                          = "Random",
#                     "RoundRobin"                      = "RoundRobin",
#                     "LeastUsed"                       = "LeastUsed",
#                     "LeastUsedDegradation"            = "LeastUsedDeg.",
#                     "PriorityLeastUsed"               = "Prio.LeastUsed",
#                     "PriorityLeastUsedDegradation"    = "Prio.LeastUsedDeg.",
#                     "PriorityLeastUsedDPF"            = "Prio.LeastUsedDPF",
#                     "PriorityLeastUsedDegradationDPF" = "Prio.LeastUsedDeg.DPF"
                   )
   return(policies)
}


# ###### Provide alignment setting for xtable() #############################
getAlignment <- function(data)
{
   alignment <- "l|"
   for(i in seq(1, ncol(data))) {
      alignment <- paste(sep="", alignment, "c|")
   }
   return(alignment)
}


# ###### Text sanitiser for xtable(): some LaTeX markup #####################
markup.latex <- function(text)
{
   return(gsub(" -> ", " $\\rightarrow$ ",
             gsub("UPARROW", "$\\uparrow$",
                gsub("DOWNARROW", "$\\downarrow$",
                   gsub(">=", "$\\ge$",
                      gsub("%", "\\%",
                         gsub("Q([0-9]+)", "Q$_{\\\\mathrm{\\1%}}$", text, fixed = FALSE),
                         fixed = TRUE),
                      fixed = TRUE),
                   fixed = TRUE),
                fixed = TRUE),
             fixed = TRUE))
}


# ###### Text sanitiser for xtable(): some HTML markup ######################
markup.html <- function(text)
{
   return(gsub(" -> ", " → ",
             gsub(">=", "&ge;",
                gsub("Q([0-9]+)", "Q<sub>\\1%</sub>", text, fixed = FALSE),
             fixed = TRUE),
          fixed = TRUE))
}


# ###### Text sanitiser for xtable(): Use LaTeX bold text ###################
bold.latex <- function(text)
{
   return(paste(sep="", "{\\textbf{", markup.latex(text), "}}"))
}


# ###### Text sanitiser for xtable(): Use HTML bold text ####################
bold.html <- function(text) {
   return(paste(sep="", "<strong>", markup.html(text), "</strong>"))
}


# ###### Write table ########################################################
writeTable <- function(table, name, prefix, label, caption)
{
   latexOutputTable <- xtable(table,
                              align   = getAlignment(table),
                              label   = paste(sep="", "tab:", prefix, label),
                              caption = caption)
   htmlOutputTable  <- xtable(table,
                              align   = getAlignment(table),
                              label   = paste(sep="", "tab:", prefix, label),
                              caption = caption)

   latexName <- paste(sep="", name, "-", prefix, "-", label, ".tex")
   print(latexOutputTable,
         size                       = "footnotesize",
         include.rownames           = FALSE,
         sanitize.colnames.function = bold.latex,
         sanitize.text.function     = markup.latex,
         table.placement            = NULL,
         caption.placement          = "top",
         hline.after                = c(-1, 0, seq(0, nrow(latexOutputTable))),
         floating.environment       = "table*",
         NA.string                  = "--",
         file                       = latexName,
         type                       = "latex")
   cat(sep="", "Wrote ", latexName, "\n")

   htmlName <- paste(sep="", name, "-", prefix, "-", label, ".html")
   print(htmlOutputTable,
         include.rownames           = FALSE,
         table.placement            = NULL,
         caption.placement          = "top",
         sanitize.colnames.function = bold.html,
         hline.after                = c(-1, 0, seq(0, nrow(htmlOutputTable))),
         NA.string                  = "--",
         file                       = htmlName,
         type                       = "html")
   cat(sep="", "Wrote ", htmlName, "\n")
}


# ###### Plot results #######################################################
systemSummaryTable <- function(name, prefix)
{
   calcAppPETotalUsedCapacity   <- readResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalUsedCapacity.data.bz2"))
   calcAppPETotalWastedCapacity <- readResults(paste(sep="/", name, "lan.calcAppPoolElementArray.calcAppServer-CalcAppPETotalWastedCapacity.data.bz2"))

   calcAppPUAverageHandlingTime <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageHandlingTime.data.bz2"))
   calcAppPUAverageQueuingDelay <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageQueuingDelay.data.bz2"))
   calcAppPUAverageStartupDelay <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageStartupDelay.data.bz2"))
   calcAppPUAverageProcessingTime <- readResults(paste(sep="/", name, "lan.calcAppPoolUserArray.calcAppQueuingClient-CalcAppPUAverageProcessingTime.data.bz2"))

   # ====== Calculate utilisation ===========================================
   utilisation <- data.table(utilisation = 100.0 * calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity /
                               (calcAppPETotalUsedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalUsedCapacity + calcAppPETotalWastedCapacity$lan.calcAppPoolElementArray.calcAppServer.CalcAppPETotalWastedCapacity))
   calcAppPETotalUsedCapacity <- cbind(calcAppPETotalUsedCapacity, utilisation)


   # ====== Aggregate utilisation ===========================================
   PUs <- c(50)   # c(20, 50, 80)

   aggregation <- calcAppPETotalUsedCapacity %>%
                     filter(scenarioNumberOfCalcAppPoolUsersVariable %in% PUs) %>%
                     select(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy, lan, utilisation) %>%
                     group_by(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy, lan) %>%
                     summarise(.groups         = "keep",
                               utilisationMean = mean(utilisation))

   aggregationMEC <- aggregation %>% filter(lan == 1) %>% rename(utilisationMeanMEC = utilisationMean)
   aggregationPMC <- aggregation %>% filter(lan == 2) %>% rename(utilisationMeanPMC = utilisationMean)
   aggregation <- merge(aggregationMEC, aggregationPMC, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                     group_by(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy) %>%
                     select(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy, utilisationMeanMEC, utilisationMeanPMC)


   # ====== Aggregate times ===========================================
   prep <- function(data, key) {
      return(data %>%
                filter(scenarioNumberOfCalcAppPoolUsersVariable %in% PUs) %>%
                select(c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy", all_of(key))) %>%
                group_by(scenarioNumberOfCalcAppPoolUsersVariable, calcAppPoolElementSelectionPolicy) %>%
                summarise(.groups = "keep",
                          time    =  mean(eval(parse(text=key)), na.rm = TRUE)
                         )
            )
   }

   a <- prep(calcAppPUAverageQueuingDelay, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay") %>%
           rename("Queuing Time" = time)
   b <- prep(calcAppPUAverageStartupDelay, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay") %>%
           rename("Startup Time" = time)
   c <- prep(calcAppPUAverageProcessingTime, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime") %>%
           rename("Processing Time" = time)
   d <- prep(calcAppPUAverageHandlingTime, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageHandlingTime") %>%
           rename("Handling Time" = time)

   combined <- aggregation %>%
                  merge(a, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(b, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(c, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(d, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  mutate(calcAppPoolElementSelectionPolicy = getPolicyAbbreviations(calcAppPoolElementSelectionPolicy)) %>%
                  rename("Clients" = scenarioNumberOfCalcAppPoolUsersVariable,
                         "Policy"  = calcAppPoolElementSelectionPolicy,
                         "Util.~Fog"     = utilisationMeanMEC,
                         "Util.~Cloud"   = utilisationMeanPMC)  %>%
                  select(!Clients)   # !!!


   return (combined)
}


for(setup in c("2hr", "1hr")) {

   systemSummaryO <- systemSummaryTable(paste(sep="", "mec2-", setup, "O/Results"), paste(sep="", "mec2-", setup, "O"))
   systemSummaryP <- systemSummaryTable(paste(sep="", "mec2-", setup, "P/Results"), paste(sep="", "mec2-", setup, "P"))


   updown <- function(values)
   {
      return( ifelse(values > 0, sprintf("UPARROW~%1.2f", as.numeric(unlist(values))),
                                 sprintf("DOWNARROW~%1.2f", abs(as.numeric(unlist(values))))) )
   }

   s <- systemSummaryO
   cols <- c("Util.~Fog", "Util.~Cloud", "Queuing Time", "Startup Time", "Processing Time", "Handling Time")
   for(col in cols) {
      s[col] <- updown(systemSummaryO[col] - systemSummaryP[col])
   }
   s <- s %>% select(!c("Queuing Time", "Startup Time"))

   writeTable(s,
              paste(sep="", "mec2-", setup, "P/Results"),
              paste(sep="", "MEC2-", setup, "P"),
              "Comparison",
              paste(sep="", gsub("hr", "-Hour", setup), ": Comparison of prediction relative to original results"))
}
