#ifndef AABB_H_20110714
#define AABB_H_20110714

#include "math3d.h"
#include "ray.h"
#include "interval.h"

#include <limits>

namespace picogen { namespace cracker {

class BoundingBox {
public:
        // Use max() instead of infinity() to allow for integer types.
        BoundingBox ()
        : minimum_ (
                std::numeric_limits<real>::max(),
                std::numeric_limits<real>::max(),
                std::numeric_limits<real>::max())
        , maximum_ (
                -std::numeric_limits<real>::max(),
                -std::numeric_limits<real>::max(),
                -std::numeric_limits<real>::max())
        {
        }


        BoundingBox (Point const &center, real width, real height, real depth)
                : minimum_(center - Vector(width/2,height/2,depth/2))
                , maximum_(center + Vector(width/2,height/2,depth/2))
        {}


        BoundingBox (Point const & p)
        : minimum_ (p), maximum_ (p)
        {
        }



        BoundingBox (Point const & a, Point const & b)
        : minimum_ (cracker::min(a,b))
        , maximum_ (cracker::max(a,b))
        {
        }

        // New naming scheme.
        Point min () const { return minimum_; }
        Point max () const { return maximum_; }

        real width ()  const { return maximum_.x() - minimum_.x(); }
        real height () const { return maximum_.y() - minimum_.y(); }
        real depth ()  const { return maximum_.z() - minimum_.z(); }

        real size (unsigned int axis) const {
                return maximum_[axis] - minimum_[axis];
        }
        real center (unsigned int axis) const {
                return maximum_[axis]*real(0.5)
                     + minimum_[axis]*real(0.5);
        }
        Point center() const {
                return lerp (minimum_, maximum_, 0.5);
        }

        void reset() {
                *this = BoundingBox();
        }

        /*void translate (real x, real y, real z) {
                minimum_.x += x;
                minimum_.y += y;
                minimum_.z += z;
                maximum_.x += x;
                maximum_.y += y;
                maximum_.z += z;
        }*/

        real centerX() const {
                return 0.5 * minimum_.x() + 0.5 * maximum_.x();
        }
        real centerY() const {
                return 0.5 * minimum_.y() + 0.5 * maximum_.y();
        }
        real centerZ() const {
                return 0.5 * minimum_.z() + 0.5 * maximum_.z();
        }

        real minX() const { return minimum_.x(); }
        real minY() const { return minimum_.y(); }
        real minZ() const { return minimum_.z(); }

        real maxX() const { return maximum_.x(); }
        real maxY() const { return maximum_.y(); }
        real maxZ() const { return maximum_.z(); }

private:
        Point minimum_, maximum_;

};



inline
BoundingBox merge ( // <-- "union" was already occupied by someone.
        BoundingBox const &box,
        Point       const &p
) {
        return BoundingBox (min (box.min(), p),
                            max (box.max(), p));
}

inline
BoundingBox merge ( // <-- "union" was already occupied by someone.
        BoundingBox const &lhs,
        BoundingBox const &rhs
) {
        return BoundingBox (min (lhs.min(), rhs.min()),
                            max (lhs.max(), rhs.max()));
}

inline
bool overlap (BoundingBox const &lhs, BoundingBox const &rhs) {
        const Point &lmin = lhs.min(), &lmax = lhs.max(),
                    &rmin = rhs.min(), &rmax = rhs.max();
        const bool x  = (lmax.x() >= rmin.y())
                      & (lmin.x() <= rmax.x());
        const bool y  = (lmax.y() >= rmin.y())
                      & (lmin.y() <= rmax.y());
        const bool z  = (lmax.z() >= rmin.z())
                      & (lmin.z() <= rmax.z());
        return x & y & z;
}


inline
bool inside (BoundingBox const &box, Point const &point) {
        const Point &min = box.min();
        const Point &max = box.max();

        const bool x  = (point.x() >= min.x())
                      & (point.x() <= max.x());
        const bool y  = (point.y() >= min.y())
                      & (point.y() <= max.y());
        const bool z  = (point.z() >= min.z())
                      & (point.z() <= max.z());
        return x & y & z;
}



inline real volume (BoundingBox const &box) {
        const Vector diff = box.max() - box.min();
        return diff.x() * diff.y() * diff.z();
}


// Returns the intersection of a ray with a bounding box. Note: As we
// really define "Ray" as a Half-Ray with only positive distance sign,
// only intersections with at least max>=0 are considered; if both, min and max
// are negative, no intersection is returned; if only max>=0, then the interval
// is max,max.
inline Interval::Optional intersect (Ray const & ray, BoundingBox const & box) {
        using std::swap;

        real t0 = 0;//ray.minT;
        real t1 = std::numeric_limits<real>::infinity();//ray.maxT;

        // X
        {
                real
                        i = real(1) / ray.direction().x(),
                        near = (box.min().x() - ray.origin().x()) * i,
                        far  = (box.max().x() - ray.origin().x()) * i;

                if (near > far) swap (near, far);

                t0 = near > t0 ? near : t0;
                t1 = far < t1 ? far : t1;

                if (t0 > t1) return Interval::Optional();
        }

        // Y
        {
                real
                        i = real(1) / ray.direction().y(),
                        near = (box.min().y() - ray.origin().y()) * i,
                        far  = (box.max().y() - ray.origin().y()) * i;

                if (near > far) swap (near, far);

                t0 = near > t0 ? near : t0;
                t1 = far < t1 ? far : t1;

                if (t0 > t1) return Interval::Optional();
        }

        // Z
        {
                real
                        i = real(1) / ray.direction().z(),
                        near = (box.min().z() - ray.origin().z()) * i,
                        far  = (box.max().z() - ray.origin().z()) * i;

                if (near > far) swap (near, far);

                t0 = near > t0 ? near : t0;
                t1 = far < t1 ? far : t1;

                if (t0 > t1) return Interval::Optional();
        }
        //if ((t0 < 0) & (t1 < 0)) return Interval::Optional();
        /*if (t0 < 0) {
                //if (std::numeric_limits<real>::infinity() != t1)
                        return Interval(t1,t1);
                //return Interval::Optional();
        }*/
        return Interval (t0, t1);
}

inline bool does_intersect (Ray const & ray, BoundingBox const & box) {
        using std::swap;
        real t0 = 0;//ray.minT;
        real t1 = std::numeric_limits<real>::max();

        // X
        {
                real i = real(1) / ray.direction().x();
                real near = box.min().x() - ray.origin().x() * i;
                real far  = box.max().x() - ray.origin().x() * i;

                if (near > far) swap (near, far);

                t0 = near > t0 ? near : t0;
                t1 = far < t1 ? far : t1;

                if (t0 > t1) return false;
        }

        // Y
        {
                real i = real(1) / ray.direction().y();
                real near = box.min().y() - ray.origin().y() * i;
                real far  = box.max().y() - ray.origin().y() * i;

                if (near > far) swap (near, far);

                t0 = near > t0 ? near : t0;
                t1 = far < t1 ? far : t1;

                if (t0 > t1) return false;
        }

        // Z
        {
                real i = real(1) / ray.direction().z();
                real near = box.min().z() - ray.origin().z() * i;
                real far  = box.max().z() - ray.origin().z() * i;

                if (near > far) swap (near, far);

                t0 = near > t0 ? near : t0;
                t1 = far < t1 ? far : t1;

                if (t0 > t1) return false;
        }

        return true;
}

} }

#endif // AABB_H_20110714

