#pragma once

#include <ql/timeseries.hpp>

using namespace QuantLib;

TimeSeries<Real> init() {
	double values[5] = {1.1, 1.5, 2.3, 1.7, 2.8};
	std::vector<QuantLib::Date> dates;
	dates.push_back(Date(12, Nov, 2012));
	dates.push_back(Date(13, Nov, 2012));
	dates.push_back(Date(14, Nov, 2012));
	dates.push_back(Date(15, Nov, 2012));
	dates.push_back(Date(16, Nov, 2012));

	TimeSeries<Real> series(dates.begin(), dates.end(), values);
	TimeSeries<Real> series2(Date(12, November, 2012), values, &values[5]);
	return series;
}

