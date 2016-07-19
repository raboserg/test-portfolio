#pragma once

#include "targetver.h"

#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif

#include <ql/math/matrix.hpp>
#include <ql/patterns/singleton.hpp>
#include <ql/math/statistics/statistics.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>

#include <stdio.h>
#include <tchar.h>

#include <ql/models/marketmodels/all.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>

#include <memory>
//#include <vector>

#include "PortfolioStatistics.h"
#include "PortfolioAllocationConstraints.h"
#include "EfficientFrontier.h"

