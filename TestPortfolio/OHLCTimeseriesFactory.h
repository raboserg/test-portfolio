#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <ql\quantlib.hpp>
#include <boost\algorithm\string\split.hpp>
#include <boost\algorithm\string\classification.hpp>

// Wrapper creating a QuantLib OHLC time series from *.csv file
QuantLib::TimeSeries<QuantLib::IntervalPrice> QL_OHLC(char *filename) {
	std::ifstream file(filename);
	std::string line;
	std::vector<QuantLib::Real> open, close, high, low;
	std::vector<QuantLib::Date> dates;
	std::vector<std::string> tokens;
	QuantLib::TimeSeries<QuantLib::IntervalPrice> ochl;

	while (std::getline(file, line)) {
		std::stringstream stringStream(line);
		std::string content;
		int item = 0;
		while (std::getline(stringStream, content, ',')) {
			switch (item) {
			case 0:
				boost::algorithm::split(tokens, content, boost::algorithm::is_any_of("/"));
				
				dates.push_back(QuantLib::Date(
						QuantLib::Day(std::stoi(tokens.at(0))),
						QuantLib::Month(std::stoi(tokens.at(1))),
						QuantLib::Year(std::stoi(tokens.at(2)))));
				
				break;
			case 1: open.push_back(std::stod(content)); break;
			case 2: close.push_back(std::stod(content)); break;
			case 3: high.push_back(std::stod(content)); break;
			case 4: low.push_back(std::stod(content)); break;
			}
			item++;
		}
	}
	return QuantLib::IntervalPrice::makeSeries(dates, open, high, low, close);
}

auto testQL_OHLC() -> int {
	// Function call
	char* file = argv[1];
	QuantLib::TimeSeries<QuantLib::IntervalPrice> ohlc = QL_OHLC(file);

	// Start date of the time series
	std::cout << "Start date: " << ohlc.firstDate() << std::endl;

	// Is the time series empty?
	std::cout << "Time series empty (1:Yes; 0:No): " << ohlc.empty() << std::endl;

	// Return the number of dates/time series length
	std::cout << "Series length (number of time steps): " << ohlc.size() << std::endl;

	// Access the OHLC values
	std::vector<QuantLib::IntervalPrice> v;
	for (auto i : ohlc)
		v.push_back(i.second);
	std::cout << "\nOpen\tHigh\tLow\tClose" << std::endl;
	for (auto i : v)
		std::cout << i.open() << "\t" << i.high() << "\t" << i.low() << "\t" << i.close() << std::endl;
	std::cout << "\n";
	return 0;
}