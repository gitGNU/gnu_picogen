#ifndef GRID_TERRAIN_H_20110715
#define GRID_TERRAIN_H_20110715

#include "real.h"
#include "math3d/vector.h"
#include <vector>

namespace picogen { namespace cracker {

class PotentialIntersection;
class Ray;
class Normal;

class GridTerrain {
public:
        GridTerrain ();
        PotentialIntersection operator() (Ray const &ray) const;
private:
        Normal normal(real centerH, real x, real z) const;
        real height (int x, int y) const;
private:
        Vector size_;
        unsigned int heightfieldWidth_, heightfieldDepth_;
        std::vector<real> heightfield_;
        real pixelWidth_, pixelDepth_;
};

} }

#endif // GRID_TERRAIN_H_20110715
