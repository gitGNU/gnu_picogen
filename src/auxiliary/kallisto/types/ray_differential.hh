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

#ifndef RAYDIFFERENTIAL_H_INCLUDED_20090311
#define RAYDIFFERENTIAL_H_INCLUDED_20090311

namespace kallisto {
        template <typename point_t, typename direction_t>
        struct ray_differential_t {
                ray_t<point_t, direction_t> ray;
                bool hasDifferentials;
                ray_t<point_t, direction_t> rx, ry;
        };
}



namespace kallisto {
        template <typename point_t, typename direction_t>
        class RayDifferential
        : public Ray<point_t, direction_t> {
        public:

                bool hasDifferentials;
                Ray<point_t, direction_t> rx, ry;

                RayDifferential ()
                : Ray<point_t, direction_t> ()
                , hasDifferentials(false)
                , rx(), ry()
                {}

                RayDifferential (point_t const &pos, direction_t const &dir)
                : Ray<point_t, direction_t> (pos, dir)
                , hasDifferentials(false)
                , rx(), ry()
                {}

                RayDifferential (Ray<point_t, direction_t> const & ray)
                : Ray<point_t, direction_t> (ray)
                , hasDifferentials(false)
                , rx(), ry()
                {}

                RayDifferential (RayDifferential const & rhs)
                : Ray<point_t, direction_t> (rhs)
                , hasDifferentials(rhs.hasDifferentials)
                , rx(rhs.rx)
                , ry(rhs.ry)
                {
                }

                RayDifferential & operator = (RayDifferential const & rhs) {
                        Ray<point_t, direction_t>::operator = (rhs);
                        hasDifferentials = rhs.hasDifferentials;
                        rx = rhs.rx;
                        ry = rhs.ry;
                        return *this;
                }

                RayDifferential getOffset (direction_t const &ofs) const {
                        RayDifferential ret = *this;
                        ret.position += ofs;
                        ret.rx.position += ofs;
                        ret.ry.position += ofs;
                        return ret;
                }
        };
}



namespace kallisto { namespace unit_test {
        class test_ray_differential_t {
        private:
                const static test_ray_differential_t test;
                test_ray_differential_t () {
                        UNIT_TEST_REPORT_BEGIN("");
                        UNIT_TEST_WARN("no tests defined!");
                }
        };
} }

#endif // RAYDIFFERENTIAL_H_INCLUDED_20090311
