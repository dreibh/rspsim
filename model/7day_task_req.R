library("anytime")
library("data.table")
library("ggplot2")
suppressMessages(library("dplyr"))

data <- fread("7day_task_req.csv") %>%
           mutate(Timestamp = anytime(Timestamp) - anytime(min(Timestamp))) %>%
           rename(Requests = Task_req) %>%
           mutate(Timestamp = Timestamp / (3600))

write.table(data, file="requests.csv",
            sep="\t", col.names=FALSE, row.names=FALSE, quote=FALSE)


cairo_pdf("7day_task_req.pdf",
          width=18, height=6, family="Helvetica", pointsize=32)

p <- ggplot(data) +
        theme(title            = element_text(size=32),
              plot.title       = element_text(size=20, hjust = 0.5, face="bold"),
              axis.title       = element_text(size=20, face="bold"),
              strip.text       = element_text(size=18, face="bold"),
              axis.text.x      = element_text(size=16, angle=90, hjust=1.0, face="bold", colour="black"),
              axis.text.y      = element_text(size=16, angle=90, hjust=0.5, colour="black"),
              legend.title     = element_blank(),
              legend.text      = element_text(size=18, face="bold"),
              legend.position  = "bottom",
              # panel.grid.major = element_line(size=0.5,  linetype="solid", color="lightgray"),
              # panel.grid.minor = element_line(size=0.25, linetype="solid", color="lightgray")
              # strip.background = element_blank(),
              panel.spacing    = unit(1, "cm"),
              panel.grid.major = element_line(size=0.4, colour = "black"),
              panel.grid.minor = element_line(size=0.2, colour = "gray"),
              panel.background = element_rect(fill="white", color="black", size=2)
             ) +
        labs(x = "Time [h]",
             y = "Requests") +
        geom_step(aes(x=Timestamp, y=Requests)) +
        # A line for mean value:
        geom_hline(yintercept=mean(data$Requests), linetype="solid", color="red", size=2) +
        geom_hline(yintercept=quantile(data$Requests, 0.10), linetype="dashed", color="red", size=1.5) +
        geom_hline(yintercept=quantile(data$Requests, 0.90), linetype="dashed", color="red", size=1.5) +
        # Ticks:
        scale_x_continuous(breaks=seq(0,168,24),
                           minor_breaks=seq(0,168,3)) +
        scale_y_continuous(limits=c(floor(min(data$Requests)) - 0.2, ceiling(max(data$Requests))))
print(p)

dev.off()


print(t.test(data$Requests))
