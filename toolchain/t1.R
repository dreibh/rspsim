library("anytime")
library("data.table")
library("plyr",  warn.conflicts = FALSE)
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")
library("xtable")


# ###### Read results file ###################################################
readResults <- function(name)
{
   cat(sep="", "Reading ", name, " ...\n")
   data <- fread(name, sep="\t", quote='"')
   colnames(data) <- gsub("-", ".", colnames(data))

   # print(sort(colnames(data)))
   return(data)
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


   name <- "mec2-1hrO/Results"
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
   PUs <- c(50)

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
# #    a <- calcAppPUAverageHandlingTime %>% select("lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageHandlingTime")

   a <- prep(calcAppPUAverageQueuingDelay, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageQueuingDelay") %>%
           rename(AvgQueuingTime = time)
   b <- prep(calcAppPUAverageStartupDelay, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageStartupDelay") %>%
           rename(AvgStartupSpeed = time)
   c <- prep(calcAppPUAverageProcessingTime, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageProcessingTime") %>%
           rename(AvgProcessingTime = time)
   d <- prep(calcAppPUAverageHandlingTime, "lan.calcAppPoolUserArray.calcAppQueuingClient.CalcAppPUAverageHandlingTime") %>%
           rename(AvgHandlingTime = time)

   combined <- aggregation %>%
                  merge(a, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(b, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(c, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  merge(d, by=c("scenarioNumberOfCalcAppPoolUsersVariable", "calcAppPoolElementSelectionPolicy")) %>%
                  rename("Clients" = scenarioNumberOfCalcAppPoolUsersVariable,
                         "Policy"  = calcAppPoolElementSelectionPolicy,
                         "Util. Fog"     = utilisationMeanMEC,
                         "Util. Cloud"   = utilisationMeanPMC)


