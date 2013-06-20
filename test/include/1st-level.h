#include "2nd-level.h"

// prototype for apple driver
int first_level_function(int);

int first_level_function(int i)
{
    return second_level_function(i);
}
