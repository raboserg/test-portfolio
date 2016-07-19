// TestPortfolio.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

template <class T>
class csv_istream_iterator : public iterator<input_iterator_tag, T>
{
	istream * _input;
	char _delim;
	string _value;
public:
	csv_istream_iterator(char delim = ',') : _input(0), _delim(delim) {}
	csv_istream_iterator(istream & in, char delim = ',') : _input(&in), _delim(delim) { ++*this; }

	const T operator *() const {
		istringstream ss(_value);
		T value;
		ss >> value;
		return value;
	}

	istream & operator ++() {
		if (!(getline(*_input, _value, _delim)))
		{
			_input = 0;
		}
		return *_input;
	}

	bool operator !=(const csv_istream_iterator & rhs) const {
		return _input != rhs._input;
	}
};

template <>
const string csv_istream_iterator<string>::operator *() const {
	return _value;
}

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


template <>
const string string_istream_iterator<string>::operator *() const {
	return _value;
}

class Formatter
{
	enum { WIDTH = 4, COLUMNS = 8 };

	string _delim;

	long _count;
public:
	Formatter(string delim = ",") : _delim(delim), _count(0) {}

	template<class T>
	string operator ()(const T & t)
	{
		ostringstream out;
		if (_count) {
			out << _delim;
			if (!(_count % COLUMNS)) out << endl;
		}
		out << hex << setw(WIDTH) << t;
		++_count;
		return out.str();
	}
};

void readFromFile() {
	
	{ // test for integers
		vector<double> v;

		ifstream fin("d:\\MY\\Data\\output_file.txt");

		if (fin) {
			copy(csv_istream_iterator<double>(fin),
				csv_istream_iterator<double>(),
				insert_iterator< vector<double> >(v, v.begin()));
			
			//copy(v.begin(), v.end(), ostream_iterator<string>(cout));
			
			/*copy(csv_istream_iterator<double>(fin),
				csv_istream_iterator<double>(),
				ostream_iterator<double>(cout, ","));*/

			fin.close();
		}
		
		if (fin) {
			transform(csv_istream_iterator<double>(fin),
				csv_istream_iterator<double>(),
				ostream_iterator<string>(cout),
				Formatter());
			fin.close();
		}
		//copy(v.begin(), v.end(), ostream_iterator<string>(cout, "\n"));
	}

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

		}
		_file.close();
	}
	testEfficientFrontier();
	//copy(v.begin(), v.end(), ostream_iterator<string>(cout, "\n"));
	
}

int main()
{
	//testPortfolioAllocationConstraints();
	readFromFile();
	testEfficientFrontier();
	return 0;
}
