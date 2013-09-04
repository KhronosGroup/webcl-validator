#ifndef WEBCLVALIDATOR_WEBCLDEBUG
#define WEBCLVALIDATOR_WEBCLDEBUG

//#define DEBUG 1

#ifdef DEBUG
#undef DEBUG
#include <iostream>
#define DEBUG(x) do { x } while (0)
#else
#undef DEBUG
#define DEBUG(x) ;
#endif

#endif
