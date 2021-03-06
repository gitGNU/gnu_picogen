//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2009  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "ignore_strict_aliasing" // because of boost::optional

#include "../../include/primitives/closedsphere.hh"
#include "../../include/sampling.hh"
#include "../../include/basictypes/intersection.hh"

#include "../../include/bxdf/lambertian.hh"
#include "../../include/bxdf/mirror.hh"

namespace picogen { namespace redshift { namespace primitive {



ClosedSphere::ClosedSphere (Point const & center, real_t radius)
: sphereData (center, radius)
{
}



ClosedSphere::~ClosedSphere () {
}



BoundingBox ClosedSphere::boundingBox () const {
        return sphereData.boundingBox();
}



bool ClosedSphere::doesIntersect (Ray const &ray) const {
        return sphereData.doesIntersect (ray);
}



optional<Intersection>
 ClosedSphere::intersect(Ray const &ray) const {

        const optional<DifferentialGeometry> i(sphereData.intersect(ray));
        if (i) {
                return Intersection (*this, *i);
        } else {
                return false;
        }
}



shared_ptr<Bsdf> ClosedSphere::getBsdf(
        const DifferentialGeometry & dgGeom
) const {
        shared_ptr<Bsdf> bsdf (new Bsdf(dgGeom));
        bsdf->add (shared_ptr<Bxdf>(new bsdf::Mirror (Color::FromRGB(0.75,0.5,0.25,ReflectanceSpectrum))));
        bsdf->add (shared_ptr<Bxdf>(new bsdf::Lambertian (Color(0.5))));
        return bsdf;
}



} } }
