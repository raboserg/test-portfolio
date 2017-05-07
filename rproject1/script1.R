source("D:/MY/### TRADING ###/R files/Utils.R")

www <- "http://marketdata.websol.barchart.com/getHistory.csv?key=02cf5b8702c22f4f94cf8a98183c706e&symbol=GOOG&type=daily&startDate=20001001000000"
GOOG <- read.table(www, header = TRUE, sep = ",", row.names = NULL)
GOOG.ts.close <- GOOG[, 7]
GOOG.ts.date <- as.Date(GOOG[, 3])
GOOG.ts <- ts(GOOG.ts.date, GOOG.ts.close)
GOOG.ts.ts <- data.frame(GOOG.ts.date, GOOG.ts.close)
#plot(GOOG.ts.ts)

www <- "http://marketdata.websol.barchart.com/getHistory.csv?key=02cf5b8702c22f4f94cf8a98183c706e&symbol=AAPL&type=daily&startDate=20001001000000"
AAPL <- read.table(www, header = TRUE, sep = ",", row.names = NULL)
AAPL.ts.close <- AAPL[, 7]
AAPL.ts.date <- as.Date(AAPL[, 3])
AAPL.ts <- ts(AAPL.ts.date, AAPL.ts.close)
AAPL.ts.ts <- data.frame(AAPL.ts.date, AAPL.ts.close)
#plot(AAPL.ts.ts)

#library(tseries)
#GOOG.data <- get.hist.quote('GOOG', start = "2004-01-01")
#GOOG.price <- GOOG.data$Close
#AAPL.data <- get.hist.quote('AAPL', start = "2004-01-01")
#AAPL.price <- AAPL.data$Close
#EPAM.data <- get.hist.quote('EPAM', start = "2004-01-01")
#EPAM.price <- EPAM.data$Close

oddcount(GOOG.price, AAPL.price)