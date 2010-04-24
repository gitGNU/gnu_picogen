//
//    quatsch - a tiny compiler framework for strict functional languages
//
//    Copyright (C) 2010  Sebastian Mach
//
//      email: a@b.c, with a=phresnel, b=gmail, c=com
//        www: http://phresnel.org
//             http://picogen.org
//
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the  Free  Software  Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed  in the hope that it will be useful, but
//    WITHOUT  ANY  WARRANTY;   without   even  the  implied  warranty  of
//    MERCHANTABILITY  or  FITNESS FOR A PARTICULAR  PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy  of  the  GNU General Public License
//    along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef HEIGHTMAPDEF_HH_INCLUDED_20100424
#define HEIGHTMAPDEF_HH_INCLUDED_20100424

#include <string>
#include <sstream>
#include <fstream>

#include <boost/filesystem.hpp>

#include "heightmap.hh"

namespace quatsch {  namespace configurable_functions {

template <typename FUNCTION, typename COMPILER>
Heightmap <FUNCTION, COMPILER> :: Heightmap (
        ::std::map<std::string,std::string>& static_parameters,
        ::std::vector <FunctionPtr>& runtime_parameters
) {

        {
                using namespace quatsch::backend::est;
                if (runtime_parameters.size() < 2)
                        throw insufficient_number_of_operands_exception (2);
                if (runtime_parameters.size() > 2)
                        throw too_many_operands_exception (2);

                ufun = runtime_parameters[0];
                vfun = runtime_parameters[1];
        }
        /*::Heightmap (std::map<std::string,std::string> &parameters, BasicFunction *ufun, BasicFunction *vfun)
        : ufun(ufun), vfun(vfun), noiseEvalFun(0), persistenceFun(0)
        //, enableBilinearNoiseMapFilter(true)
        , filter (cosine)
        , noiseDepth(6)
        , frequency(2)*/

        using namespace std;
        typedef map<string,string> Map;


        std::string filename;

        //====---- - - -  -   -    -      -
        // Scan Parameters.
        //====---- - - -  -   -    -      -
        string nonExistantParameterNames (""); // To collect together parameter names that don't exist, for dumping errors in the end.

        for (Map::const_iterator it=static_parameters.begin();
             it!=static_parameters.end();
             ++it
        ) {
                const string name = it->first;
                /// \todo Add some shorter Mnenomics.
                if (name == string("width")) {
                        istringstream hmmm (static_parameters[name]);
                        hmmm >> width;
                } else if (name == string("height") ) {
                        istringstream hmmm (static_parameters[name]);
                        hmmm >> height;
                } else if (name == string("depth")) {
                        istringstream hmmm (static_parameters[name]);
                        hmmm >> depth;
                } else if (name == string("filename")) {
                        istringstream hmmm (static_parameters[name]);
                        hmmm >> filename;
                } else if (name == string("filter")) {
                        istringstream hmmm (static_parameters[name]);
                        string filterType;
                        hmmm >> filterType;
                        if (filterType == "bilinear") {
                                filter = bilinear;
                        } else if (filterType == "nearest") {
                                filter = nearest;
                        } else if (filterType == "cosine") {
                                filter = cosine;
                        } else if (filterType == "cubic") {
                                filter = cubic;
                        } else {
                                throw general_exception ("Heightmap: unknown filter type for 'filter': '" + filterType + "' (only 'bilinear','nearest','cosine' are supported)");
                        }
                } else {
                        nonExistantParameterNames += (nonExistantParameterNames!=""?", ":"") + string("'") + it->first + string("'");
                }
        }

        // Any parameters set that we don't know?
        if (nonExistantParameterNames != "") {
                throw general_exception ("the following parameters do not exist for 'LayeredNoise': "+nonExistantParameterNames);
        }


        // TODO: Check filename ...
        {
                /*
                std::ifstream ifs (filename.c_str());
                if (!ifs) ... // <-- fails for directories
                */
                if (!boost::filesystem::exists(filename) || !boost::filesystem::is_regular_file(filename))
                        throw general_exception ("the file \"" + filename + "\" could not be opened for reading.");
        }

        if (!heightmap.load (filename, redshift::aux::Average))
                throw general_exception ("error while loading \"" + filename + "\" (possibly: out of memory, unsupported image format)");
}


template <typename FUNCTION, typename COMPILER>
Heightmap <FUNCTION, COMPILER> :: ~Heightmap() {
}




template <typename FUNCTION, typename COMPILER>
typename Heightmap <FUNCTION, COMPILER>::scalar_t
Heightmap <FUNCTION, COMPILER>::operator () (
        const typename Heightmap <FUNCTION, COMPILER>::parameters_t& parameters
) const {
        const scalar_t u = (*ufun) (parameters);
        const scalar_t v = (*vfun) (parameters);

        switch (filter) {
        case nearest: return heightmap.nearest(u, v);
        case bilinear: return heightmap.lerp(u, v);
        case cosine: return heightmap.cosine(u, v);
        case cubic: return heightmap.cubic(u, v);
        }
        return 0.0;
}

} }

#endif // HEIGHTMAPDEF_HH_INCLUDED_20090813
