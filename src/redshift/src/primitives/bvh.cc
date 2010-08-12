//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010  Sebastian Mach (*1983)
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

#include "../../include/constants.hh"
#include "../../include/basictypes/intersection.hh"
#include "../../include/primitives/bvh.hh"


namespace redshift { namespace primitive {

//-----------------------------------------------------------------------------
// Node
//-----------------------------------------------------------------------------
struct BvhNode {
        std::vector<shared_ptr<BoundPrimitive> > primitives;
        scoped_ptr<BvhNode> childA, childB;
        BoundingBox boundingBox;

        typedef std::vector<shared_ptr<BoundPrimitive> >::iterator It;
        typedef std::vector<shared_ptr<BoundPrimitive> >::const_iterator CIt;

        void add (shared_ptr<BoundPrimitive> prim) {
                primitives.push_back (prim);
        }

        void compile() {
                //std::cout << "BvhNode::compile()" << std::endl;

                //std::cout << "  primitive-count: " << primitives.size() << std::endl;
                if (primitives.size() == 1) {
                        //std::cout << "  is done here" << std::endl;
                        return;
                }

                boundingBox.reset();
                for (It it = primitives.begin(); it!=primitives.end(); ++it) {
                        boundingBox = merge (boundingBox,
                                             ((**it).boundingBox()));
                }

                //std::cout << "  boundingBox.size   = {" << boundingBox.width() << ", " << boundingBox.height() << ", " << boundingBox.depth() << "}" << std::endl;
                //std::cout << "  boundingBox.center = {" << boundingBox.center(0) << ", " << boundingBox.center(1) << ", " << boundingBox.center(2) << "}" << std::endl;

                int splitAxis_ = 0;
                if (boundingBox.size(1) > boundingBox.size(splitAxis_)) splitAxis_ = 1;
                if (boundingBox.size(2) > boundingBox.size(splitAxis_)) splitAxis_ = 2;
                const int splitAxis = splitAxis_;

                const real_t center = boundingBox.center(splitAxis);

                childA.reset(new BvhNode);
                childB.reset(new BvhNode);

                for (It it = primitives.begin(); it!=primitives.end(); ++it) {
                        if ((**it).boundingBox().center(splitAxis) < center) {
                                childA->add(*it);
                        } else {
                                childB->add(*it);
                        }
                }

                primitives.clear();

                if (!childA->primitives.empty()) childA->compile();
                else childA.reset();

                if (!childB->primitives.empty()) childB->compile();
                else childB.reset();
        }

        bool doesIntersect (Ray const &ray) const {
                if (!does_intersect<false>(ray, boundingBox))
                        return false;
                if (childA && childA->doesIntersect(ray))
                        return true;
                if (childB && childB->doesIntersect(ray))
                        return true;

                for (CIt it=primitives.begin(); it!=primitives.end(); ++it) {
                        if ((*it)->doesIntersect(ray))
                                return true;
                }

                return false;
        }

        optional<Intersection> intersect(Ray const &ray) const {
                real_t nearest = constants::real_max, tmp;
                optional<Intersection> nearestI, tmpI;
                for (CIt it=primitives.begin();
                        it!=primitives.end(); ++it
                ) {
                        if ((tmpI=(*it)->intersect (ray))
                           && (tmp=tmpI->getDistance()) < nearest
                        ) {
                                nearest = tmp;
                                nearestI = tmpI;
                        }
                }

                if (childA) {
                        if ((tmpI = childA->intersect(ray))
                         && (tmp = tmpI->getDistance()) < nearest) {
                                nearest = tmp;
                                nearestI = tmpI;
                        }
                }
                if (childB) {
                        if ((tmpI = childB->intersect(ray))
                         && (tmp = tmpI->getDistance()) < nearest) {
                                nearest = tmp;
                                nearestI = tmpI;
                        }
                }
                return nearestI;
        }
};


//-----------------------------------------------------------------------------
// Bvh
//-----------------------------------------------------------------------------
Bvh::Bvh(shared_ptr<BvhNode> root) : root(root) {
}



Bvh::~Bvh() {
}



BoundingBox Bvh::boundingBox() const {
        return root->boundingBox;
}



bool Bvh::doesIntersect (Ray const &ray) const {
        return root->doesIntersect(ray);
}



optional<Intersection> Bvh::intersect(Ray const &ray) const {
        return root->intersect(ray);
}



//-----------------------------------------------------------------------------
// Builder
//-----------------------------------------------------------------------------
BvhBuilder::BvhBuilder() : root(new BvhNode) {
}



void BvhBuilder::add (shared_ptr<BoundPrimitive> prim) {
        root->add(prim);
}


shared_ptr<Bvh> BvhBuilder::toBvh() {
        root->compile();
        shared_ptr<Bvh> ret = shared_ptr<Bvh>(new Bvh(root));
        root.reset();
        return ret;
}


} }
