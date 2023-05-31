#pragma once
#define INCLUDE_AFXDB
#include "..\stdafx.h"

#define BOOST_ALL_NO_LIB // disable boost "auto-link" feature
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <tesseract/baseapi.h>
#include <gsl/pointers>
#include <gsl/util>

using namespace std::literals;
