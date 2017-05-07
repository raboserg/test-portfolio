trials <- 10000 # simulation trials
T <- 1 # time until expiration (years)
m <- 200 # subintervals
dt <- T / m # time per subinterval (years)

## set interest rate model parameters
k <- 0.4
theta <- 0.05
beta <- 0.03

## set strikes
K1 <- 500
K2 <- 240

## create storage vectors
s1 <- rep(0, m + 1) # stock price path 1
s2 <- rep(0, m + 1) # stock price path 2
r <- rep(0, m + 1) # interest rate
c <- rep(0, trials) # payoff

## set initial stock prices and interest rate
s1[1] <- 500
s2[1] <- 240
r[1] <- 0.01

## begin simulation
for (j in 1:trials) {
	for (i in 2:(m + 1)) {
		z1 <- rnorm(1, 0, 1)
		z2 <- rnorm(1, 0, 1)
		z3 <- rnorm(1, 0, 1)

		ds1 <- s1[i - 1] * (r[i - 1] * dt + GOOG.vol * sqrt(dt) * z1)
		ds2 <- s2[i - 1] * (r[i - 1] * dt + AAPL.vol * sqrt(dt) * (rho * z1 + sqrt(1 - rho ^ 2) * z2))
		dr <- k * (theta - r[i - 1]) * dt + beta * sqrt(dt) * z3

		s1[i] <- s1[i - 1] + ds1
		s2[i] <- s2[i - 1] + ds2
		r[i] <- r[i - 1] + dr
	}
	ss <- sum(r[2:(m + 1)] * dt)
	c[j] <- ifelse(s1[m + 1] > K1 && s2[m + 1] > K2, exp( - ss), 0)
}

cat("Option Price Estimate:", round(mean(c), 3), "n")
cat("Standard Error:", round(sd(c) / sqrt(trials), 3), "n")