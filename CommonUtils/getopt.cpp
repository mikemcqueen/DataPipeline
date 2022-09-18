/*
Newsgroups: mod.std.unix
Subject: public domain AT&T getopt source
Date: 3 Nov 85 19:34:15 GMT

Here's something you've all been waiting for:  the AT&T public domain
source for getopt(3).  It is the code which was given out at the 1985
UNIFORUM conference in Dallas.  I obtained it by electronic mail
directly from AT&T.  The people there assure me that it is indeed
in the public domain.
*/

/*LINTLIBRARY*/
#include "stdafx.h"
#include "Log.h"

//#define nullptr	0
#define EOF	(-1)


int	optind = 1;
int	optopt;
wchar_t	*optarg;

namespace util 
{

int
getopt(int argc,
       wchar_t* argv[],
       const wchar_t* opts)
{
	static int sp = 1;
	wchar_t c;
	const wchar_t *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(wcscmp(argv[optind], L"--") == 0) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp = wcschr(opts, c)) == nullptr) {
		LogError(L"%ls: illegal option -- %c", argv[0], c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			LogError(L"%ls: option requires an argument -- %c", argv[0], c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = nullptr;
	}
	return(c);
}

}