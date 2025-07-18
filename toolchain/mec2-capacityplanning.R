cloudServers           <-  14
mecServers             <-  0

cloudServerCapacity    <- 300*60   # Work Units/s
cloudMaxProcsPerServer <-   2
mecServerCapacity      <- 450*60   # Work Units/s
mecMaxProcsPerServer   <-   3

clients                <-  50
totalReqsPerMin        <- 6.619643 / 6   # From 7day_task_req.csv: mean of 6.619643 in 6 minutes
reqSize                <- 6*90*300*60    # Work Units

totalServers        <- mecServers + cloudServers
totalReqsPerSec     <- totalReqsPerMin / 60
reqsPerClientPerMin <- totalReqsPerMin / clients
reqsPerClientPerSec <- totalReqsPerSec / clients

totalUtilisation    <- clients * reqsPerClientPerSec * reqSize / (mecServers * mecServerCapacity + cloudServers * cloudServerCapacity)
cloudlUtilisation   <- clients * reqsPerClientPerSec * reqSize / (cloudServers * cloudServerCapacity)
mecUtilisation      <- clients * reqsPerClientPerSec * reqSize / (mecServers * mecServerCapacity)

totalReqsPerWeek    <- totalReqsPerMin * 60 * 24 * 7
totalReqsPerSec     <- totalReqsPerMin / 60
totalAvailableWorkUnitsPerWeek <- (mecServers * mecServerCapacity + cloudServers * cloudServerCapacity) * 60 * 60 * 24 * 7


cat(sep="", "Servers:\t", totalServers, "\t(", cloudServers, " Cloud, ", mecServers, " MEC)\n")
cat(sep="", "Clients:\t", clients, "\n")
cat(sep="", "\n")

cat(sep="", "Requests per Client:           \t", reqsPerClientPerSec, " Requests/s = ", reqsPerClientPerMin, " Requests/min\n")
cat(sep="", "Inter-Request Time per Client: \t", 1 / reqsPerClientPerSec, " s = ", 1 / reqsPerClientPerMin, " min\n")
cat(sep="", "\n")

cat(sep="", "Total Requests per Week:            \t", totalReqsPerWeek, " Requests\n")
cat(sep="", "Total Requested Work Units per Week:\t", totalReqsPerWeek * reqSize, " Work Units\n")
cat(sep="", "Total Available Work Units per Week:\t", totalAvailableWorkUnitsPerWeek, " Work Units\n")
cat(sep="", "\n")

cat(sep="", "Utilisation Total:     \t", 100.0 * totalUtilisation,  " %\n")
cat(sep="", "Utilisation Cloud-only:\t", 100.0 * cloudlUtilisation, " %\n")
cat(sep="", "Utilisation MEC-only:  \t", 100.0 * mecUtilisation,    " %\n")
cat(sep="", "\n")

cat(sep="", "Duration for 1 Work Unit at Full Capacity in Cloud:\t", 3600 / cloudServerCapacity, " s = ", 60 / cloudServerCapacity, " min\n")
cat(sep="", "Duration for 1 Work Unit at Full Capacity in MEC:  \t", 3600 / mecServerCapacity,   " s = ", 60 / mecServerCapacity,   " min\n")
cat(sep="", "\n")

cat(sep="", "Duration for 1 Request (", reqSize, " Work Units) at Full Capacity in Cloud:   \t", reqSize / cloudServerCapacity, " s\n")
cat(sep="", "Duration for 1 Request (", reqSize, " Work Units) at Minimum Capacity in Cloud:\t", reqSize / (cloudServerCapacity / cloudMaxProcsPerServer), " s\n")
cat(sep="", "Duration for 1 Request (", reqSize, " Work Units) at Full Capacity in MEC:     \t", reqSize / mecServerCapacity, " s\n")
cat(sep="", "Duration for 1 Request (", reqSize, " Work Units) at Minimum Capacity in MEC:  \t", reqSize / (mecServerCapacity / mecMaxProcsPerServer), " s\n")



# variable <- 2500
# gamma    <- 4
#
# minCapacity <- variable / (1 + ((gamma -1) / 2))
# maxCapacity <- gamma * minCapacity
#
# cat(sep="\t", variable, minCapacity, maxCapacity, "\n")
# print(mean(runif(10000, minCapacity, maxCapacity)))
