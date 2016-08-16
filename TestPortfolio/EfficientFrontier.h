#pragma once
#include <iostream> 
#include <cstdlib>
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <ql/quantlib.hpp>
#include <boost/format.hpp>
#include <functional>
#include <numeric>
#include <fstream>

namespace {
	using namespace QuantLib;

	double calculatePortfolioReturn(const double proportionA, const double expectedReturnA, const double expectedReturnB) {
		return proportionA * expectedReturnA + (1 - proportionA) * expectedReturnB;
	}

	Volatility calculatePortfolioRisk(const double proportionA, const Volatility volatilityA, const Volatility volatilityB, const double covarianceAB) {
		return std::sqrt(
			std::pow(proportionA, 2) * std::pow(volatilityA, 2) + std::pow(1 - proportionA, 2) * std::pow(volatilityB, 2) + (2 * proportionA * (1 - proportionA) * covarianceAB)
		);
	}

	//BOOST_AUTO_TEST_CASE(testEfficientFrontier) {
	void testEfficientFrontier(const Matrix covarianceMatrix, const Matrix portfolioReturnVector) {

		
		/*
		Matrix covarianceMatrix(4, 4);
									  //row 1
		covarianceMatrix[0][0] = .40; //AAPL-AAPL
		covarianceMatrix[0][1] = .05; //AAPL-IBM
		covarianceMatrix[0][2] = .02; //AAPL-ORCL
		covarianceMatrix[0][3] = .04; //AAPL-GOOG
									  //row 2
		covarianceMatrix[1][0] = .05; //IBM-AAPL
		covarianceMatrix[1][1] = .20; //IBM-IBM 
		covarianceMatrix[1][2] = .01; //IBM-ORCL
		covarianceMatrix[1][3] = -.06; //IBM-GOOG
									   //row 3
		covarianceMatrix[2][0] = .02; //ORCL-AAPL 
		covarianceMatrix[2][1] = .01; //ORCL-IBM
		covarianceMatrix[2][2] = .30; //ORCL-ORCL
		covarianceMatrix[2][3] = .03; //ORCL-GOOG
									  //row 4
		covarianceMatrix[3][0] = .04; //GOOG-AAPL
		covarianceMatrix[3][1] = -.06; //GOOG-IBM
		covarianceMatrix[3][2] = .03; //GOOG-ORCL
		covarianceMatrix[3][3] = .15; //GOOG-GOOG
		*/

		std::cout << "Covariance matrix of returns: " << std::endl 
			<< covarianceMatrix << std::endl;

		Size _size = portfolioReturnVector.size1();

		//portfolio return vector         
		/*
		Size _size = 4;
		Matrix portfolioReturnVector(_size, 1);
		portfolioReturnVector[0][0] = .10; //AAPL
		portfolioReturnVector[1][0] = .03; //IBM
		portfolioReturnVector[2][0] = .07; //ORCL
		portfolioReturnVector[3][0] = .08; //GOOG
		*/
		std::cout << "Portfolio return vector" << std::endl 
			<< portfolioReturnVector << std::endl;

		//constant 
		Rate c = .05;

		//portfolio return vector minus constant rate
		Matrix portfolioReturnVectorMinusC(_size, 1);
		for (int i = 0; i < _size; ++i) {
			portfolioReturnVectorMinusC[i][0] = portfolioReturnVector[i][0] - c;
		}

		std::cout << boost::format("Portfolio return vector minus constantrate (c = %f)") % c << std::endl 
			<< portfolioReturnVectorMinusC << std::endl;

		//inverse of covariance matrix
		const Matrix& inverseOfCovarMatrix = inverse(covarianceMatrix);

		//z vectors
		const Matrix& portfolioAz = inverseOfCovarMatrix * portfolioReturnVector;
		std::cout << "Portfolio A z vector" << std::endl;
		std::cout << portfolioAz << std::endl;
		double sumOfPortfolioAz = 0.0;
		std::for_each(portfolioAz.begin(), portfolioAz.end(), [&](Real n) {
			sumOfPortfolioAz += n;
		});

		const Matrix& portfolioBz = inverseOfCovarMatrix * portfolioReturnVectorMinusC;
		std::cout << "Portfolio B z vector" << std::endl;
		std::cout << portfolioBz << std::endl;
		double sumOfPortfolioBz = 0.0;
		std::for_each(portfolioBz.begin(), portfolioBz.end(), [&](Real n) {
			sumOfPortfolioBz += n;
		});

		//portfolio weights
		Matrix weightsPortfolioA(_size, 1);
		for (int i = 0; i < _size; ++i) {
			weightsPortfolioA[i][0] = portfolioAz[i][0] / sumOfPortfolioAz;
		}

		std::cout << "Portfolio A weights" << std::endl 
			<< weightsPortfolioA << std::endl;

		Matrix weightsPortfolioB(_size, 1);
		for (int i = 0; i < _size; ++i) {
			weightsPortfolioB[i][0] = portfolioBz[i][0] / sumOfPortfolioBz;
		}

		std::cout << "Portfolio B weights" << std::endl 
			<< weightsPortfolioB << std::endl;

		//portfolio risk and return
		const Matrix& expectedReturnPortfolioAMatrix = transpose(weightsPortfolioA) * portfolioReturnVector;
		double expectedReturnPortfolioA = expectedReturnPortfolioAMatrix[0][0];
		const Matrix& variancePortfolioAMatrix = transpose(weightsPortfolioA) * covarianceMatrix * weightsPortfolioA;
		double variancePortfolioA = variancePortfolioAMatrix[0][0];
		double stdDeviationPortfolioA = std::sqrt(variancePortfolioA);
		std::cout << boost::format("Portfolio A expected return: %f") % expectedReturnPortfolioA << std::endl;
		std::cout << boost::format("Portfolio A variance: %f") % variancePortfolioA << std::endl;
		std::cout << boost::format("Portfolio A standard deviation: %f") % stdDeviationPortfolioA << std::endl;

		const Matrix& expectedReturnPortfolioBMatrix = transpose(weightsPortfolioB) * portfolioReturnVector;
		double expectedReturnPortfolioB = expectedReturnPortfolioBMatrix[0][0];
		const Matrix& variancePortfolioBMatrix = transpose(weightsPortfolioB) * covarianceMatrix * weightsPortfolioB;
		double variancePortfolioB = variancePortfolioBMatrix[0][0];
		double stdDeviationPortfolioB = std::sqrt(variancePortfolioB);
		std::cout << boost::format("Portfolio B expected return: %f") % expectedReturnPortfolioB << std::endl;
		std::cout << boost::format("Portfolio B variance: %f") % variancePortfolioB << std::endl;
		std::cout << boost::format("Portfolio B standard deviation: %f") % stdDeviationPortfolioB << std::endl;

		//covariance and correlation of returns
		const Matrix& covarianceABMatrix = transpose(weightsPortfolioA) * covarianceMatrix * weightsPortfolioB;
		double covarianceAB = covarianceABMatrix[0][0];
		double correlationAB = covarianceAB / (stdDeviationPortfolioA * stdDeviationPortfolioB);
		std::cout << boost::format("Covariance of portfolio A and B: %f") % covarianceAB << std::endl;
		std::cout << boost::format("Correlation of portfolio A and B: %f") % correlationAB << std::endl;

		//generate envelope set of portfolios
		double startingProportion = -.40;
		double increment = .10;
		std::map<double, std::pair<Volatility, double> > mapOfProportionToRiskAndReturn;
		std::map<Volatility, double> mapOfVolatilityToReturn;
		for (int i = 0; i < 21; ++i) {
			double proportionA = startingProportion + i * increment;
			Volatility riskEF = calculatePortfolioRisk(proportionA, stdDeviationPortfolioA, stdDeviationPortfolioB, covarianceAB);
			double returnEF = calculatePortfolioReturn(proportionA, expectedReturnPortfolioA, expectedReturnPortfolioB);
			mapOfProportionToRiskAndReturn[proportionA] = std::make_pair(riskEF, returnEF);
			mapOfVolatilityToReturn[riskEF] = returnEF;
		}

		//write data to a file
		std::ofstream envelopeSetFile;
		envelopeSetFile.open("/tmp/envelope.dat", std::ios::out);
		for (std::map<double, std::pair<Volatility, double> >::const_iterator i = mapOfProportionToRiskAndReturn.begin(); i != mapOfProportionToRiskAndReturn.end(); ++i) {
			envelopeSetFile << boost::format("%f %f %f") % i->first % i->second.first % i->second.second << std::endl;
		}
		envelopeSetFile.close();

		//find minimum risk portfolio on efficient frontier
		std::pair<Volatility, double> minimumVariancePortolioRiskAndReturn = *mapOfVolatilityToReturn.begin();
		Volatility minimumRisk = minimumVariancePortolioRiskAndReturn.first;
		double maximumReturn = minimumVariancePortolioRiskAndReturn.second;
		std::cout << boost::format("Maximum portfolio return for risk of %f is %f") % minimumRisk % maximumReturn << std::endl;

		//generate efficient frontier
		std::map<Volatility, double> efficientFrontier;
		for (std::map<double, std::pair<Volatility, double> >::const_iterator i = mapOfProportionToRiskAndReturn.begin(); i != mapOfProportionToRiskAndReturn.end(); ++i) {
			efficientFrontier[i->second.first] = i->second.second;
			if (i->second.first == minimumRisk) break;
		}

		//write efficient frontier to file
		std::ofstream efFile;
		efFile.open("/tmp/ef.dat", std::ios::out);
		for (std::map<Volatility, double>::const_iterator i = efficientFrontier.begin(); i != efficientFrontier.end(); ++i) {
			efFile << boost::format("%f %f") % i->first % i->second << std::endl;
		}
		efFile.close();

		//plot with gnuplot using commands below. Run 'gnuplot' then type in: 
		/*
		set key top left
		set key box
		set xlabel "Volatility"
		set ylabel "Expected Return"
		plot '/tmp/envelope.dat' using 2:3 with linespoints title "Envelope", '/tmp/ef.dat' using 1:2  w points pointtype 5 t "Efficient Frontier"
		*/

	}
}