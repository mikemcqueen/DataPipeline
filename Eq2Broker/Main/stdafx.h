#pragma once
#define INCLUDE_AFXDB
#include "..\stdafx.h"

#pragma warning(push)
#pragma warning(disable:4996)
//#include <boost/algorithm/string.hpp>
#pragma warning(pop)
//#include <boost/algorithm/string/predicate.hpp>

#define BOOST_ALL_NO_LIB // disable boost "auto-link" feature
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <tesseract/baseapi.h>
#include <gsl/pointers>
#include <gsl/util>
