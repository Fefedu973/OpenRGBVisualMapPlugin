#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "math.h"

class MathUtils {

public:

    static unsigned int FloorToNearestTen(unsigned int num)
    {
        return (num/10)*10;
    }

};

#endif // MATHUTILS_H
