library("anytime")
library("data.table")
suppressMessages(library("dplyr"))

data <- fread("7day_task_req.csv") %>%
           mutate(Timestamp = anytime(Timestamp) - anytime(min(Timestamp))) %>%
           rename(Requests = Task_req)

write.table(data, file="requests.csv",
            sep="\t", col.names=FALSE, row.names=FALSE, quote=FALSE)
