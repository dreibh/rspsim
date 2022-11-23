library("anytime")
library("data.table")
library("ggplot2")
suppressMessages(library("dplyr"))

data <- fread("7day_task_req.csv") %>%
           mutate(Timestamp = anytime(Timestamp) - anytime(min(Timestamp))) %>%
           rename(Requests = Task_req) %>%
           mutate(Timestamp = Timestamp / (24*3600))



write.table(data, file="requests.csv",
            sep="\t", col.names=FALSE, row.names=FALSE, quote=FALSE)

p <- ggplot(data) +
  theme(#legend.justification = legendJustification,
        #legend.position      = "bottom",
        axis.ticks           = element_line(colour = "black"),
        panel.grid.major     = element_line(colour = "blue", size=0.25),
        panel.grid.minor     = element_line(colour = "blue3", size=0.125),
        panel.border         = element_rect(colour = "black", fill = NA),
        legend.background    = element_rect(colour = "blue",  fill = "#ffffaa", size=1)) +
  labs(x = "Time",
       y = "Requests") +
  geom_step(aes(x=Timestamp, y=Requests))
print(p)
