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

#ifndef WATER_HH_20101217
#define WATER_HH_20101217



#include <string>
#include <actuarius/bits/enum.hh>
#include "redshift/include/smart_ptr.hh"

#include "cosyfloat.hh"
#include "vector3d.hh"
#include "stash.hh"
#include "terrain.hh"



namespace picogen { namespace cosyscene {
class WaterFitting : public StashableMixin<WaterFitting>
{
public:
        WaterFitting();

        void setSeaLevel (CosyFloat);
        CosyFloat seaLevel() const;

        bool data_equals(WaterFitting const &rhs) const ;

        template <typename Arch> void serialize (Arch &arch);
private:
        CosyFloat seaLevel_;
};
} }



namespace picogen { namespace cosyscene {
class Water
{
public:
        Water() ;

        redshift::shared_ptr<TerrainFormation> formation() const;
        redshift::shared_ptr<Material>  material() const;
        redshift::shared_ptr<WaterFitting> fitting() const;

        template<typename Arch>
        void serialize (Arch &arch);

private:
        redshift::shared_ptr<TerrainFormation> formation_;
        redshift::shared_ptr<Material> material_;
        redshift::shared_ptr<WaterFitting> fitting_;
};
} }



#endif // WATER_HH_20101217
