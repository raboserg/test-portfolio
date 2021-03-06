#pragma once
#include <iostream> 
#include <cstdlib>
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <ql/quantlib.hpp>
#include <boost/format.hpp>
#include <boost/range/numeric.hpp>
#include <functional>
#include <numeric>
#include <fstream>
#include <vector>

namespace {

	using namespace QuantLib;

	//aggregates all constraint expressions into a single constraint
	class PortfolioAllocationConstraints : public Constraint {

	public:

		PortfolioAllocationConstraints(const std::vector<std::function<bool(const Array&)> >& expressions):
			Constraint(boost::shared_ptr<PortfolioAllocationConstraints::Impl>(new PortfolioAllocationConstraints::Impl(expressions))) {}

	private:
		// constraint implementation
		class Impl : public Constraint::Impl {
		public:

			Impl(const std::vector<std::function<bool(const Array&)> >& expressions) :
				expressions_(expressions) {}

			bool test(const Array& x) const {
				for (auto iter = expressions_.begin(); iter < expressions_.end(); ++iter) {
					if (!(*iter)(x)) {
						return false;
					}
				}
				//will only get here if all constraints satisfied
				return true;
			}

		private:

			const std::vector<std::function<bool(const Array&)> > expressions_;
		};
	};


	class ThetaCostFunction : public CostFunction {

	public:
		ThetaCostFunction(const Matrix& covarianceMatrix, const Matrix& returnMatrix, const int sizeProportions) :
				covarianceMatrix_(covarianceMatrix), returnMatrix_(returnMatrix), sizeProportions_(sizeProportions) {}

		Real value(const Array& proportions) const {
			QL_REQUIRE(proportions.size() == sizeProportions_, "Four assets in portfolio!");
			Array allProportions((sizeProportions_ + 1));
			copy(proportions.begin(), proportions.end(), allProportions.begin());
			allProportions[sizeProportions_] = 1 - boost::accumulate(proportions, 0.0);
			/*
			allProportions[0] = proportions[0];
			allProportions[1] = proportions[1];
			allProportions[2] = proportions[2];
			allProportions[3] = 1 - (proportions[0] + proportions[1] + proportions[2]);
			*/
			return -1 * ((portfolioMean(allProportions) - c_) / portfolioStdDeviation(allProportions));
		}

		Disposable<Array> values(const Array& proportions) const {
			QL_REQUIRE(proportions.size() == sizeProportions_, "Four assets in portfolio!");
			Array values(1);
			values[0] = value(proportions);
			return values;
		}

		void setC(Real c) { c_ = c; }

		Real getC() const { return c_; }

		Real portfolioMean(const Array& proportions) const {
			Real portfolioMean = (proportions * returnMatrix_)[0];
			//std::cout << boost::format("Portfolio mean: %.4f") % portfolioMean << std::endl;
			return portfolioMean;
		}

		Real portfolioStdDeviation(const Array& proportions) const {
			Matrix matrixProportions(sizeProportions_ + 1, 1);
			copy(proportions.begin(), proportions.end(), matrixProportions.row_begin(0));
			const Matrix& portfolioVarianceMatrix = transpose(matrixProportions) * covarianceMatrix_ * matrixProportions;
			Real portfolioVariance = portfolioVarianceMatrix[0][0];
			Real stdDeviation = std::sqrt(portfolioVariance);
			//std::cout << boost::format("Portfolio standard deviation: %.4f") % stdDeviation << std::endl;
			return stdDeviation;
		}

	private:
		const Matrix& covarianceMatrix_;
		const Matrix& returnMatrix_;
		int sizeProportions_;
		Real c_;
	};

	void testPortfolioAllocationConstraints(Matrix covarianceMatrix, Matrix portfolioReturnVector) {
	//BOOST_AUTO_TEST_CASE(testNoShortSales) {

/*
		Matrix covarianceMatrix(4, 4);
		//row 1
		covarianceMatrix[0][0] = .1; //AAPL-AAPL
		covarianceMatrix[0][1] = .03; //AAPL-IBM
		covarianceMatrix[0][2] = -.08; //AAPL-ORCL
		covarianceMatrix[0][3] = .05; //AAPL-GOOG
									  //row 2
		covarianceMatrix[1][0] = .03; //IBM-AAPL
		covarianceMatrix[1][1] = .20; //IBM-IBM 
		covarianceMatrix[1][2] = .02; //IBM-ORCL
		covarianceMatrix[1][3] = .03; //IBM-GOOG
									  //row 3
		covarianceMatrix[2][0] = -.08; //ORCL-AAPL 
		covarianceMatrix[2][1] = .02; //ORCL-IBM
		covarianceMatrix[2][2] = .30; //ORCL-ORCL
		covarianceMatrix[2][3] = .2; //ORCL-GOOG
									 //row 4
		covarianceMatrix[3][0] = .05; //GOOG-AAPL
		covarianceMatrix[3][1] = .03; //GOOG-IBM
		covarianceMatrix[3][2] = .2; //GOOG-ORCL
		covarianceMatrix[3][3] = .9; //GOOG-GOOG
		*/
		std::cout << "Covariance matrix of returns: " << std::endl;
		std::cout << covarianceMatrix << std::endl;
		/*
		//portfolio return vector         
		Matrix portfolioReturnVector(4, 1);
		portfolioReturnVector[0][0] = .08; //AAPL
		portfolioReturnVector[1][0] = .09; //IBM
		portfolioReturnVector[2][0] = .10; //ORCL
		portfolioReturnVector[3][0] = .11; //GOOG
		*/
		std::cout << "Portfolio return vector" << std::endl;
		std::cout << portfolioReturnVector << std::endl;

		//constraints
		std::vector<std::function<bool(const Array&)> >  noShortSalesConstraints(2);
		//constraints implemented as C++ 11 lambda expressions
		noShortSalesConstraints[0] = [](const Array& x) {
			Real x4 = 1.0 - boost::accumulate(x, 0.0);
			return (std::count_if(x.begin(), x.end(), [](int i) {return i >= 0.0; }) == x.size() && x4 >= 0.0);
		};
		//constraints implemented as C++ 11 lambda expressions
		noShortSalesConstraints[1] = [](const Array& x) {
			//Real x4 = 1.0 - (x[0] + x[1] + x[2]); 
			Real sum = boost::accumulate(x, 0.0);
			Real x4 = 1.0 - sum;
			return 1.0 - (sum + x4) < 1e-9;
		};

		//instantiate constraints
		PortfolioAllocationConstraints noShortSalesPortfolioConstraints(noShortSalesConstraints);

		const Size maxIterations = 100000;
		const Size minStatIterations = 100;
		const Real rootEpsilon = 1e-9;
		const Real functionEpsilon = 1e-9;
		const Real gradientNormEpsilon = 1e-9;
		EndCriteria endCriteria(maxIterations, minStatIterations, rootEpsilon, functionEpsilon, gradientNormEpsilon);

		std::map<Rate, std::pair<Volatility, Real> > mapOfStdDeviationToMeanNoShortSales;

		const Rate startingC = -.035;
		const Real increment = .005;

		const Size sizeProportions = portfolioReturnVector.size1() - 1;
		const Real valueProportions = 1 / (static_cast<Real>(sizeProportions) + 1); /*.2500 = 1/4*/
		ThetaCostFunction thetaCostFunction(covarianceMatrix, portfolioReturnVector, sizeProportions);

		for (int i = 0; i < 40; ++i) {
			
			Rate c = startingC + (i * increment);

			thetaCostFunction.setC(c);
			Problem efficientFrontierNoShortSalesProblem(thetaCostFunction, 
				noShortSalesPortfolioConstraints, 
				Array(sizeProportions, valueProportions));
			Simplex solver(.01);
			EndCriteria::Type noShortSalesSolution = solver.minimize(efficientFrontierNoShortSalesProblem, endCriteria);
			std::cout << boost::format("Solution type: %s") % noShortSalesSolution << std::endl;

			const Array& results = efficientFrontierNoShortSalesProblem.currentValue();
			Size sizeResult = results.size();
			Array proportions(sizeResult + 1);
			copy(results.begin(), results.end(), proportions.begin());
			proportions[results.size()] = 1 - boost::accumulate(results, 0.0);
			/*
			proportions[0] = results[0];
			proportions[1] = results[1];
			proportions[2] = results[2];
			proportions[3] = 1.0 - (results[0] + results[1] + results[2]);
			*/
			std::cout << boost::format("Constant (c): %.4f") % thetaCostFunction.getC() << std::endl;
			copy(proportions.begin(), proportions.end(), ostream_iterator<Real>(cout, "\n"));
			/*
			std::cout << boost::format("AAPL weighting: %.4f") % proportions[0] << std::endl;
			std::cout << boost::format("IBM weighting: %.4f") % proportions[1] << std::endl;
			std::cout << boost::format("ORCL weighting: %.4f") % proportions[2] << std::endl;
			std::cout << boost::format("GOOG weighting: %.4f") % proportions[3] << std::endl;
			*/
			std::cout << boost::format("Theta: %.4f") % (-1 * efficientFrontierNoShortSalesProblem.functionValue()) << std::endl;
			Real portfolioMean = thetaCostFunction.portfolioMean(proportions);
			std::cout << boost::format("Portfolio mean: %.4f") % portfolioMean << std::endl;
			Volatility portfolioStdDeviation = thetaCostFunction.portfolioStdDeviation(proportions);
			std::cout << boost::format("Portfolio standard deviation: %.4f") % portfolioStdDeviation << std::endl;
			mapOfStdDeviationToMeanNoShortSales[c] = std::make_pair(portfolioStdDeviation, portfolioMean);
			std::cout << "------------------------------------" << std::endl;
		}

		//write efficient frontier with no short sales to file
		std::ofstream noShortSalesFile;
		noShortSalesFile.open("/tmp/noshortsales.dat", std::ios::out);
		for (std::map<Rate, std::pair<Volatility, Real> >::const_iterator i = mapOfStdDeviationToMeanNoShortSales.begin(); i != mapOfStdDeviationToMeanNoShortSales.end(); ++i) {
			noShortSalesFile << boost::format("%f %f %f") % i->first % i->second.first % i->second.second << std::endl;
		}
		noShortSalesFile.close();

		//plot with gnuplot using commands below Run 'gnuplot' then type in: 
		/*
		set terminal png
		set output "/tmp/noshortsales.png"
		set key top left
		set key box
		set xlabel "Volatility"
		set ylabel "Expected Return"
		plot '/tmp/noshortsales.dat' using 2:3 w linespoints title "No Short Sales"
		*/
	}
}