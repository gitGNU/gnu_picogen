//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010 Sebastian Mach (*1983)
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

#ifndef LAZY_QUADTREE_HH_INCLUDED_20100116
#define LAZY_QUADTREE_HH_INCLUDED_20100116


#include "../basictypes/spectrum.hh"
#include "../primitives/primitive.hh"

namespace picogen { namespace redshift {
        class Intersection;
        class Scene;
        class HeightFunction;
} }


namespace picogen { namespace redshift { namespace primitive {

        class LazyQuadtreeImpl;

        SEALED(LazyQuadtree);

        class LazyQuadtree
                : public Primitive
                , MAKE_SEALED(LazyQuadtree)
        {
        public:
                LazyQuadtree(
                        shared_ptr<HeightFunction const> fun, real_t size,
                        unsigned int maxRecursion,
                        real_t lodFactor,
                        Color color
                );
                ~LazyQuadtree ();

                bool doesIntersect (Ray const &ray) const;

                optional<Intersection> intersect(Ray const &ray) const;

                shared_ptr<Bsdf> getBsdf(
                        const DifferentialGeometry & dgGeom) const;

                void prepare (const Scene &scene) ;

                void prune () ;
                void setCurrentScanline (unsigned int) ;

        private:

                LazyQuadtree();
                LazyQuadtree(LazyQuadtree const&);
                LazyQuadtree &operator = (LazyQuadtree const&);


                shared_ptr<LazyQuadtreeImpl> impl;
                shared_ptr<HeightFunction const> heightFun;
        };
} } }

#endif // LAZY_QUADTREE_HH_INCLUDED_20100116
