#ifndef PRIMARYDISTANCE_H_20110708
#define PRIMARYDISTANCE_H_20110708

#include "ray.h"
#include "scene.h"
#include "real.h"
#include "color.h"

#include<cassert>

namespace picogen { namespace cracker {

class Random;
class RendererBase;

class PrimaryDistanceIntegrator {
public:
        PrimaryDistanceIntegrator () : min_(0), range_(100) {}

        PrimaryDistanceIntegrator (real min, real max)
                : min_(min), range_(max-min)
        {
                assert(min<max);
        }

        Color operator() (Ray const &,
                          Intersection const &i,
                          Scene const &,
                          RendererBase const &,
                          Random &) const
        {
                const real
                      dist = i.distance(),
                      gray = 1 - (dist - min_) / range_,
                      gray_sat = gray < 0 ? 0 :
                                 gray > 1 ? 1 :
                                 gray;
                return Color::FromRgb (gray_sat, gray_sat, gray_sat);
        }

private:
        real min_, range_;
};

} }

#endif // PRIMARYDISTANCE_H_20110708
