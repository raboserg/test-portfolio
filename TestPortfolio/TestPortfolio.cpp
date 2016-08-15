// TestPortfolio.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

template <class T>
class string_istream_iterator : public iterator<input_iterator_tag, T>
{
	istringstream* _input;
	char _delim;
	string _value;
public:
	string_istream_iterator(char delim = ',') : _input(0), _delim(delim) {}
	string_istream_iterator(istringstream & in, char delim = ',') : _input(&in), _delim(delim) { ++*this; }
	//???string_istream_iterator(const string in, char delim = ',') : _input(new istringstream()), _delim(delim) {_input->str(in);++*this;}

	const T operator *() const {
		istringstream ss(_value);
		T value;
		ss >> value;
		return value;
	}

	istream & operator ++() {
		if (!(getline(*_input, _value, _delim))) {
			_input = 0;
		}
		return *_input;
	}

	bool operator !=(const string_istream_iterator & rhs) const {
		return _input != rhs._input;
	}
};

void readDataFromFile() {

	PortfolioStatistics::instance().initialize(300);

	map<string, vector<double> > table;
	ifstream _file("d:\\MY\\Data\\output_file.txt");
	if (_file.is_open()) {
		string line;
		while (getline(_file, line)) {
			string _name;
			vector<double> row;
			istringstream ss(line);
			getline(ss, _name, ',');
			copy(string_istream_iterator<double>(ss), 
				string_istream_iterator<double>(), 
				insert_iterator<vector<double> >(row, row.begin()));
			table.insert(pair<string, vector<double> >(_name, row));
			PortfolioStatistics::instance().addSequenceRow(row);

		}
		_file.close();
	}

	int index = 0;
	//portfolio return vector         
	Matrix portfolioReturnVector(table.size(), 1);
	/*
	copy(table.begin(), table.end(), portfolioReturnVector.row_begin(0));
	cout << portfolioReturnVector << std::endl;
	*/
	for (map<string, vector<double> >::const_iterator i = table.begin(); i != table.end(); ++i) {
		portfolioReturnVector[index++][0] = i->second[0];
	}
	
	testEfficientFrontier(PortfolioStatistics::instance().covariance(), portfolioReturnVector);
	//testPortfolioAllocationConstraints(PortfolioStatistics::instance().covariance(), portfolioReturnVector);
	
	//copy(v.begin(), v.end(), ostream_iterator<string>(cout, "\n"));
	
}

int main()
{
	readDataFromFile();
	return 0;
}

 