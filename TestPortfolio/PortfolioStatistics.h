#ifndef RoCStatistics_h
#define RoCStatistics_h

#include "stdafx.h"

using namespace QuantLib;
using namespace std;

//#########################################################
class PortfolioStatistics : public Singleton<PortfolioStatistics>{
	friend class Singleton<PortfolioStatistics>;
private:
	PortfolioStatistics() :count(false), lengthSeries(0) {}
	bool count;
	int lengthSeries;
	SequenceStatistics statistics;
	vector<vector<Real> > rocSeries;

	SequenceStatistics getStatistics(){
		if(!count){
			statistics.reset();
			// read from vector and overturn
			Size size = rocSeries.size();
			for(int j = 0; j < lengthSeries; j++){
				vector<Real> tmp(size);
				for(int i = 0; i < size; i++){
					tmp[i] = rocSeries[i][j];
				}
				statistics.add(tmp);
			}
			count = true;
		}
		return statistics;
	}

	//
	Matrix getWeights(){
		Disposable<Matrix> cov = getStatistics().covariance();
		cov *= 2;
		Matrix invM(cov.rows()+1, cov.columns()+1);
		for(Size i = 0; i < cov.rows(); i++)
			for(Size j = 0; j < cov.columns(); j++)
				invM[i][j] = cov[i][j];
		for(Size i = 0; i < invM.rows(); i++)
			invM[i][invM.columns()-1] = 1;
		for(Size i = 0; i < invM.columns(); i++)
			invM[invM.rows()-1][i] = 1;
		invM[invM.rows()-1][invM.columns()-1] = 0;
		Matrix invA = inverse(invM);
		invA *= 100;
		return invA;
	}

	Real norm(const Matrix& m) {
		Real sum = 0.0;
		for (Size i=0; i<m.rows(); i++)
			for (Size j=0; j<m.columns(); j++)
				sum += m[i][j] + m[i][j];
		return sqrt(sum);
	}

public:
	//
	void initialize(const int seq_size) {
		lengthSeries = seq_size;
		clear();
	}
	//
	void clear() {
		count = false;
		statistics.reset();
		rocSeries.clear();
	}
	//
	void addSequenceRow(const vector<Real> row) {
		rocSeries.push_back(row);
	}
	//	
	Disposable<Matrix> correlation() {
		return getStatistics().correlation();
	}
	//
	Disposable<Matrix> covariance() {
		return getStatistics().covariance();
	}
	//
	vector<Real> standardDeviation() {
		return getStatistics().standardDeviation();
	}
	
	// Calculate portfolio Weights
	int getPortfolioWeights(Real *arr){
		Matrix weidghts = getWeights();
		if (weidghts.size1() != 0 && weidghts.size2() != 0) {
			for (Size i = 0; i < weidghts.rows() - 1; i++)
				arr[i] = weidghts[i][weidghts.columns() - 1];
			return 0;
		}
		return -1;
	}
	
	
	Volatility calculatePortfolioRisk() {
		double risk = 0;
		vector<Real> weightsValues;
		Matrix weidghts = getWeights();
		Matrix covariance = getStatistics().covariance();
		Size sizeRow = covariance.rows();
		Size sizeColumns = covariance.columns();
		for (Size i = 0; i < sizeRow; i++) {
			weightsValues.push_back(weidghts[i][weidghts.columns() - 1]);
		}
		vector<Real> tmp;
		for (Size i = 0; i < sizeRow; i++) {
			Real sum = 0;
			for (Size j = 0; j < sizeColumns; j++)
				sum += weightsValues[j] * covariance[i][j];
			tmp.push_back(sum);
		}
		for (Size j = 0; j < sizeColumns; j++)
			risk += tmp[j] * weightsValues[j];
		return risk;
	}

	// Calculate portfolio risk
	double portfolioRisk(Real *arr){
		getPortfolioWeights(arr);
		return calculatePortfolioRisk();
	}
};

#endif