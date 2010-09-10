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


#ifndef TERRAIN_SERIALIZATION_HH_20100902
#define TERRAIN_SERIALIZATION_HH_20100902

#include "../terrain.hh"
#include "cosyscene/serialization/stash.ser.hh"

namespace cosyscene {

template<typename Arch>
inline void QuatschSource::serialize (Arch &arch) {
        using actuarius::pack;
        arch & pack("code", code_);
}

template<typename Arch>
inline void Terrain::serialize (Arch &arch) {
        using actuarius::pack;
        if (Arch::deserialize || !stash_.empty())
                arch & pack("stash", stash_);

        arch & pack("type", kind_, Typenames);

        switch (kind_) {
        case QuatschSource: arch & pack ("parameters", quatschSource_); break;
        case None: break;
        }
}

} // namespace cosyscene

#endif // TERRAIN_SERIALIZATION_HH
