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

#ifndef HEIGHTMAPRENDERER_HH_INCLUDED_20090426
#define HEIGHTMAPRENDERER_HH_INCLUDED_20090426

#include "../sealed.hh"
#include "height-function.hh"
#include "../rendertargets/rendertarget.hh"
#include "../interaction/usercommandprocessor.hh"
#include "../interaction/progressreporter.hh"

#pragma message "is HeightmapRenderer really needed?"

namespace redshift {

        SEALED(HeightmapRenderer);

        class HeightmapRenderer : MAKE_SEALED(HeightmapRenderer) {
        public:

                HeightmapRenderer(shared_ptr<RenderTarget>,
                                  shared_ptr<HeightFunction>);
                ~HeightmapRenderer ();

                void render(interaction::ProgressReporter::Ptr,
                          interaction::UserCommandProcessor::Ptr) const ;
        private:
                // non copyable
                // TODO use NonCopyable base class instead
                HeightmapRenderer (HeightmapRenderer const &);
                HeightmapRenderer & operator= (HeightmapRenderer const &);
                HeightmapRenderer();

                shared_ptr<RenderTarget>    renderTarget;
                shared_ptr<HeightFunction>  function;
        };
}

#endif // HEIGHTMAPRENDERER_HH_INCLUDED_20090426
