#ifndef MeanVarianceFunction_h
#define MeanVarianceFunction_h
#pragma once
#include <ql/quantlib.hpp>

// mean variance objective function

namespace {

	class MeanVarianceFunction : public QuantLib::CostFunction {
	private:
		QuantLib::Matrix covariance_;
		QuantLib::Size n;

	public:
		MeanVarianceFunction(const QuantLib::Matrix& covariance)
			:covariance_(covariance)
		{
			n = covariance.rows();
		}

		QuantLib::Real value(const QuantLib::Array& x) const {

			QL_REQUIRE(x.size() == n - 1, "n – 1 weights required");

			QuantLib::Matrix weights(n, 1);
			QuantLib::Real sumWeights(0.);

			for (QuantLib::Size i = 0; i < x.size(); ++i) {
				sumWeights += x[i];
				weights[i][0] = x[i];
			}

			weights[n - 1][0] = 1 - sumWeights;

			QuantLib::Matrix var = transpose(weights) * covariance_ * weights;

			return var[0][0];
		}

		QuantLib::Disposable<QuantLib::Array> values(const QuantLib::Array& x) const {
			QuantLib::Array var(1, value(x));
			return var;
		}
	};

	class EqualRiskContributionFunction : public QuantLib::CostFunction {
	private:
		QuantLib::Matrix covariance_;
		QuantLib::Size n;

	public:
		EqualRiskContributionFunction(const QuantLib::Matrix& covariance)
			:covariance_(covariance)
		{
			n = covariance.rows();
		}

		QuantLib::Real value(const QuantLib::Array& x) const {

			QuantLib::Array dsx = values(x);
			QuantLib::Real res(0.);

			for (QuantLib::Size i = 0; i < n; ++i) {
				for (QuantLib::Size j = 0; j < i; ++j) {
					res += (dsx[i] - dsx[j]) * (dsx[i] - dsx[j]);
				}
			}

			return res;
		}

		QuantLib::Disposable<QuantLib::Array> values(const QuantLib::Array& x) const {

			QuantLib::Array dsx(n, 0.);
			for (QuantLib::Size i = 0; i < n; ++i) {
				QuantLib::Array row(covariance_.row_begin(i), covariance_.row_end(i));
				dsx[i] = QuantLib::DotProduct(row, x) * x[i];
				double test = dsx[i];
			}

			return dsx;
		}
	};

	// long only constraint
	class LongOnlyConstraint : public QuantLib::Constraint {
	private:
		class Impl : public QuantLib::Constraint::Impl {
		public:
			Impl() {}
			bool test(const QuantLib::Array& weights) const {
				QuantLib::Real sumWeights(0.);

				for (QuantLib::Size i = 0; i < weights.size(); i++) {
					if (weights[i] < 0 - GECKO::EPSILON)
						return false;
					sumWeights += weights[i];
				}

				// n'th asset weight equals 1 – sum(asset 1 …. assent n -1)
				if (1 - sumWeights < 0 - GECKO::EPSILON)
					return false;

				return true;
			}
		private:
		};
	public:
		LongOnlyConstraint()
			: QuantLib::Constraint(boost::shared_ptr(new LongOnlyConstraint::Impl)) {}
	};

	// fully invested constraint
	class FullyInvestedConstraint : public QuantLib::Constraint {
	private:
		class Impl : public QuantLib::Constraint::Impl {
		public:
			Impl() {}
			bool test(const QuantLib::Array& weights) const {
				QuantLib::Real sumWeights(0.);

				for (QuantLib::Size i = 0; i < weights.size(); i++) {
					sumWeights += weights[i];
				}

				QuantLib::Real weightN = 1. – sumWeights;

				// check minimum leverage
				if (1. –(sumWeights + weightN) < -GECKO::EPSILON)
					return false;

				return true;
			}
		private:
		};
	public:
		FullyInvestedConstraint()
			: QuantLib::Constraint(boost::shared_ptr(new FullyInvestedConstraint::Impl)) {}
	};


	class Portfolio {
	public:
		QuantLib::Matrix WeightsMV(bool isConstrained);
		QuantLib::Matrix WeightsERC();
	};

	QuantLib::Matrix Portfolio::WeightsMV(bool isConstrained)
	{

		QuantLib::Matrix weights(n, 1);

		if (isConstrained) {

			MeanVarianceFunction mvFunc(Cov());

			QuantLib::Size maxIterations = 10000;	//end search after 1000 iterations if no solution
			QuantLib::Size minStatIterations = 10;	//don’t spend more than 10 iterations at a single point
			QuantLib::Real rootEpsilon = 1e-10;	//end search if absolute difference of current and last root value is below epsilon
			QuantLib::Real functionEpsilon = 1e-10;	//end search if absolute difference of current and last function value is below epsilon
			QuantLib::Real gradientNormEpsilon = 1e-6;	//end search if absolute difference of norm of current and last gradient is below epsilon

			QuantLib::EndCriteria myEndCrit(maxIterations, minStatIterations, rootEpsilon,
				functionEpsilon, gradientNormEpsilon);

			//constraints
			LongOnlyConstraint longOnly;
			FullyInvestedConstraint fullyInvested;

			QuantLib::CompositeConstraint allConstraints(longOnly, fullyInvested);

			QuantLib::Problem myProb(mvFunc, allConstraints, QuantLib::Array(n - 1, 1. / n));

			QuantLib::Simplex solver(.05);

			QuantLib::EndCriteria::Type solution = solver.minimize(myProb, myEndCrit);

			switch (solution) {
				case QuantLib::EndCriteria::None:
				case QuantLib::EndCriteria::MaxIterations:
				case QuantLib::EndCriteria::Unknown:
					throw("#err: optimization didn’t converge – no solution found");
				default:
				;
			}

			QuantLib::Array x = myProb.currentValue();
			double sumWeights(0);
			for (QuantLib::Size i = 0; i < x.size(); ++i) {
				weights[i][0] = x[i];
				sumWeights += weights[i][0];
			}

			weights[n - 1][0] = 1 - sumWeights;

		}
		else {

			//closed form solution exists for unconstrained
			QuantLib::Matrix covInv = QuantLib::inverse(Cov());
			QuantLib::Matrix l(n, 1, 1);
			weights = (covInv * l) / (QuantLib::transpose(l) * covInv * l)[0][0];

		}

		return weights;

	}

	QuantLib::Matrix Portfolio::WeightsERC()
	{
		QuantLib::Matrix weights(n, 1);

		if (ValidateIdenticalCorrelation(correlation)) {
			weights = WeightsInvVol();

		}
		else
		{

			EqualRiskContributionFunction ercFunc(Cov());

			QuantLib::Size maxIterations = 10000;	//end search after 1000 iterations if no solution
			QuantLib::Size minStatIterations = 100;	//don't spend more than 10 iterations at a single point
			QuantLib::Real rootEpsilon = 1e-10;	//end search if absolute difference of current and last root value is below epsilon
			QuantLib::Real functionEpsilon = 1e-10;	//end search if absolute difference of current and last function value is below epsilon
			QuantLib::Real gradientNormEpsilon = 1e-6;	//end search if absolute difference of norm of current and last gradient is below epsilon

			QuantLib::EndCriteria myEndCrit(maxIterations, minStatIterations, rootEpsilon,
				functionEpsilon, gradientNormEpsilon);

			//constraints
			LongOnlyConstraint longOnly;
			FullyInvestedConstraint fullyInvested;

			QuantLib::CompositeConstraint allConstraints(longOnly, fullyInvested);

			//would be better to start with variance weighted array but easier to debug and less inputs
			QuantLib::Problem myProb(ercFunc, allConstraints, QuantLib::Array(n, 2. / n));

			QuantLib::LevenbergMarquardt solver;
			QuantLib::EndCriteria::Type solution = solver.minimize(myProb, myEndCrit);

			switch (solution) {
			case QuantLib::EndCriteria::None:
			case QuantLib::EndCriteria::MaxIterations:
			case QuantLib::EndCriteria::Unknown:
				throw("#err: optimization didn't converge – no solution found");
			default:
				;
			}

			QuantLib::Array x = myProb.currentValue();
			for (QuantLib::Size i = 0; i < x.size(); ++i)
				weights[i][0] = x[i];
		}

		return weights;

	}
}
#endif