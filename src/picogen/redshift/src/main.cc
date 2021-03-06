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

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO: check if boost reports on cerr or cout
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "../include/auxiliary/currentdate.hh"
#include "../include/auxiliary/remove_filename_extension.hh"
#include "../include/static_init.hh"

#include "../include/image-export.hh"

#include "../include/meta/compiler.hh"

//{{{{{{{{{
// Color types.
#include "basictypes/rgb.hh"


// TODO reorder

// coordinates/
#include "coordinates/uvcoordinates.hh"
#include "coordinates/imagecoordinates.hh"
#include "coordinates/lenscoordinates.hh"
#include "coordinates/texture2dcoordinates.hh"

// interaction/
#include "interaction/usercommandprocessor.hh"
#include "interaction/passivecommandprocessor.hh"
#include "interaction/progressreporter.hh"

// basictypes/
namespace redshift{class RenderTarget;}
#include "basictypes/rectangle.hh"
#include "basictypes/sample.hh"

// cameras/
#include "cameras/camera.hh"

// basictypes/
#include "basictypes/differentialgeometry.hh"
#include "basictypes/material.hh"
#include "basictypes/intersection.hh"
#include "basictypes/background.hh"
#include "basictypes/volume.hh"
#include "basictypes/film.hh"

// rendertargets/
#include "rendertargets/rendertargetlock.hh"
#include "rendertargets/rendertarget.hh"



// shapes/
#include "shapes/shape.hh"
#include "shapes/closedsphere.hh"

// primitive/
#include "primitives/primitive.hh"
//#include "primitives/heightmap.hh"
//#include "primitives/booleanfield.hh"
#include "primitives/closedsphere.hh"
#include "primitives/lazyquadtree.hh"
#include "primitives/horizonplane.hh"
#include "primitives/waterplane.hh"
#include "primitives/list.hh"

// background/
//#include "backgrounds/visualise-direction.hh"
//#include "backgrounds/monochrome.hh"
//#include "backgrounds/preetham-adapter.hh"
#include "backgrounds/pss-adapter.hh"

// Cameras.
#include "cameras/camera.hh"

// basictypes/
#include "basictypes/transport.hh"
#include "basictypes/scene.hh"
//#include "basictypes/heightmap.hh"
#include "basictypes/bsdf.hh"

// integrators/
#include "integrators/visualize-distance.hh"
#include "integrators/show-surface-normals.hh"
#include "integrators/redshift.hh"
#include "integrators/whitted.hh"
#include "integrators/emission.hh"
#include "integrators/single-scattering.hh"
#include "integrators/null.hh"
#include "integrators/path.hh"

// volume/
#include "volume/homogeneous.hh"
#include "volume/exponential.hh"
#include "volume/list.hh"


// bxdf/
#include "bxdf/lambertian.hh"

//}}}}}}}}}

#include "../include/rendertargets/sdlrendertarget.hh"
#include "../include/rendertargets/colorrendertarget.hh"
#include "../include/cameras/pinhole.hh"
#include "../include/interaction/sdlcommandprocessor.hh"
#include "../include/interaction/rendertarget-copying-reporter.hh"
#include "../include/interaction/film-to-rendertarget-copying-reporter.hh"
#include "../include/basictypes/height-function.hh"
#include "../include/basictypes/quatsch-height-function.hh"
#include "actuarius/actuarius.hh"
#include "../include/jobfile.hh"

#include <string>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/program_options.hpp>

#include "../include/auxiliary/filename_extension.hh"

namespace {
        using redshift::optional;
        using redshift::tuple;
        using redshift::make_tuple;
        using redshift::get;

        struct Options {
                bool pauseAfterRendering;
                std::string inputFile;
                std::string outputFile;
                std::string renderSetupName;
                std::string useRenderSettings;
                std::string useCamera;
                bool doSaveOutput;

                enum SaveFormat {
                        Bitmap = 0,
                        Exr = 1,
                        BitmapAndExr = 2
                } saveFormat;


                // TODO: Unsupported:
                bool printStats;
                bool headless;
        };

        // For ggc-style -f and -fno- pairs. Grabbed from
        // http://www.boost.org/doc/libs/1_42_0/doc/html/program_options/howto.html#id1420416
        std::pair<std::string, std::string> gccstyle(const std::string& s) {
                using std::string;
                if (s.find("-f") == 0) {
                        if (s.substr(2, 3) == "no-")
                                return std::make_pair(s.substr(5), string("false"));
                        else
                                return std::make_pair(s.substr(2), string("true"));
                } else {
                        return std::make_pair(string(), string());
                }
        }

        optional<Options> parseOptions (int argc, char *argv[]) {
                namespace po = boost::program_options;

                Options ret;

                /*bool print_stats, pauseAfterRendering;
                std::string input_file="", output_file;
                std::string render_setup;*/

                // Declare the supported options.
                po::positional_options_description p;
                p.add ("input-file", -1);

                po::options_description desc("Allowed options");
                desc.add_options()
                        ("help", "Print help message.")

                        ("print-stats,s",
                        po::value(&ret.printStats)
                        ->default_value(false),
                        "Print statistics after rendering.")

                        ("pause,p",
                        po::value(&ret.pauseAfterRendering)
                        ->default_value(false),
                        "Pause after rendering (if unset, the window will close "
                        "after rendering is done).")

                        ("input-file,i",
                        po::value<std::string>(&ret.inputFile),
                        "File that contains the job.")

                        /*("save-output,S",
                        po::value(&ret.doSaveOutput)
                        ->default_value(false),
                        "Save image after rendering.")*/

                        ("output-file,o",
                        po::value(&ret.outputFile),
                        "Image file to write to (supported extensions: bmp, exr, bmp+exr).")

                        ("render-settings,r",
                        po::value(&ret.useRenderSettings),
                        "If there are multiple rendering-setups in the input-file, "
                        "use this parameter to describe the title of the setup you "
                        "want to render\n"
                        "(either by [partial] name, or by zero-based "
                        "index ; if unset, you will be prompted.)"
                        )

                        ("camera,c",
                        po::value(&ret.useCamera),
                        "If there are multiple camera-setups in the input-file, "
                        "use this parameter to describe the title of the setup you "
                        "want to render \n"
                        "(either by [partial] name, or by zero-based "
                        "index ; if unset, you will be prompted.)"
                        )
                ;

                po::variables_map vm;
                try {
                        po::store(po::command_line_parser(argc, argv)
                                        .options(desc)
                                        //.extra_parser(gccstyle)
                                        .positional(p)
                                        .run()
                                , vm);
                        po::notify(vm);
                } catch (std::exception const &rhs) {
                        std::cout << rhs.what() << "\n";
                        std::cout << desc << std::endl;
                        return optional<Options>();
                }

                if (vm.count("help")) {
                        std::cout << desc << "\n";
                        return optional<Options>();
                }


                if (ret.inputFile == "") {
                        std::cout << "No input-file set.\n";
                        std::cout << desc << "\n";
                        return optional<Options>();
                }
                /*if (ret.outputFile == "" && ret.doSaveOutput) {
                        ret.outputFile = "redshift-"+CurrentDate::AsPartOfFilename()+".bmp+exr";
                        std::cout << "No output-file set, will write to '" << ret.outputFile << ".bmp+exr'.\n";
                } else if (ret.outputFile == "" && !ret.doSaveOutput) {
                }*/
                ret.doSaveOutput = ret.outputFile != "";

                const std::string ext = filename_extension (ret.outputFile);
                if (ret.doSaveOutput) {
                        if (ext == "bmp") {
                                ret.saveFormat = Options::Bitmap;
                                ret.outputFile = remove_filename_extension(ret.outputFile);
                        } else if (ext == "exr") {
                                ret.saveFormat = Options::Exr;
                                ret.outputFile = remove_filename_extension(ret.outputFile);
                        } else if (ext == "bmp+exr" || ext == "exr+bmp") {
                                ret.saveFormat = Options::BitmapAndExr;
                                ret.outputFile = remove_filename_extension(ret.outputFile);
                        } else if (ext == "" || ext == ".") {
                                std::cout << "Missing filename extension for output file\n";
                                return optional<Options>();
                        } else {
                                std::cout << "Unsupported filename extension for output file: " << ext << "\n";
                                return optional<Options>();
                        }
                }
                return ret;
        }

        tuple<int,std::string> toIntOrString (std::string const &str) {

                bool canBeNumber = true;
                for (unsigned int i=0; i<str.length(); ++i) {
                        switch (str[i]) {
                        case '0':case '1':case '2':case '3':case '4':
                        case '5':case '6':case '7':case '8':case '9':
                                break;
                        default:
                                canBeNumber = false;
                                goto afterLoop;
                        };
                }
                afterLoop:

                int num = -1;
                if (!canBeNumber) {
                        num = -1;
                } else {
                        std::stringstream ss;
                        ss << str;
                        ss >> num;
                }
                return make_tuple(num, str);
        }

        bool isWhitespaceOrEmpty (std::string const & str) {
                for (std::string::const_iterator it=str.begin();
                        it != str.end();
                        ++it
                ) {
                        if (*it != ' ' && *it != '\n' && *it != '\t' && *it != '\r')
                                return false;
                }
                return true;
        }
}

namespace parsi {
        /*
        struct ImageResolution { unsigned int width, height; };
        ImageResolution parseImageResolution (std::istream &in) {
                // int char int
        }*/
}

namespace {

        int queryRenderSettingsMatch (
                redshift_file::Scene const &scene,
                std::string const & str
        ) {
                using namespace redshift_file;
                using namespace std;

                int index = -1;
                if (isWhitespaceOrEmpty(str)) {
                        return -1;
                }

                const tuple<int,std::string> ns = toIntOrString(str);

                if (get<0>(ns)>=0 && (unsigned)get<0>(ns)<scene.renderSettingsCount()) {
                        index = get<0>(ns);
                } else {
                        for (unsigned int i=0; i<scene.renderSettingsCount(); ++i) {
                                if (scene.renderSettings(i).title == get<1>(ns)) {
                                        index = i;
                                        break;
                                } else if (index < 0) {
                                        if (std::string::npos !=
                                                scene.renderSettings(i).title.find(get<1>(ns)))
                                                index = i;
                                }
                        }
                }
                if (index < 0) {
                        std::cout << "Number or name \"" << str << "\" "
                                << "not found. Please type in a valid "
                                << "number or [partial] name." << std::endl;
                }
                return index;
        }
        void queryRenderSettings (redshift_file::Scene &scene, const Options & options) {
                using namespace redshift_file;
                using namespace std;
                if (scene.renderSettingsCount()>1) {
                        int index = -1;

                        // At first, try to match what was given as an option to us.
                        if (!isWhitespaceOrEmpty(options.useRenderSettings)) {
                                index = queryRenderSettingsMatch (scene, options.useRenderSettings);
                        }

                        // If still not, dump a menu.
                        if (index<0) {
                                std::cout << "\nThere are multiple render settings present: \n\n";
                                for (unsigned int i=0; i<scene.renderSettingsCount(); ++i) {
                                        const RenderSettings &rs = scene.renderSettings(i);
                                        std::cout << "  [" << i << "] "
                                        << setfill('.') << left << setw(16)
                                        << rs.title
                                        << resetiosflags(ios_base::adjustfield)
                                        << rs.width
                                        << "x" << rs.height
                                        << ", " << rs.samplesPerPixel
                                        << " sample" << (rs.samplesPerPixel!=1?"s":"") << " per pixel"
                                        << ", volume-render: " << VolumeIntegrator::Typenames[rs.volumeIntegrator.type]
                                        << "\n";
                                }
                                std::cout << "\nWhich one do you want to use (number or [partial] name)? "<< std::endl;

                                while (index<0) {
                                        std::string str;
                                        std::getline (std::cin,str);
                                        index = queryRenderSettingsMatch (scene, str);
                                }
                        }
                        std::cout << "You have chosen [" << index << "], \""
                                << scene.renderSettings(index).title << "\"." << std::endl;

                        RenderSettings tmp = scene.renderSettings(index);
                        scene.pruneRenderSettings();
                        scene.addRenderSettings (tmp);
                } else if (scene.renderSettingsCount() == 0) {
                        std::cout << "There are no render settings. Please consult the documentation, "
                                << "for now, we'll just use 640*480 pixels, no volume rendering"
                                << std::endl;
                        RenderSettings rs;
                        rs.width = 640;
                        rs.height = 480;
                        scene.addRenderSettings (rs);
                        /*
                        std::cout << "\nThere are no render settings. I'll guide you to one.\n" << std::flush;

                        optional<ImageResolution> res;
                        do {
                                std::cout << "Please enter an image resolution in format \"<width>x<height>\": " << std::endl;
                        } */
                }
        }


        // Stupid code dup from above.
        int queryCameraMatch (
                redshift_file::Scene const &scene,
                std::string const & str
        ) {
                using namespace redshift_file;
                using namespace std;
                int index = -1;

                if (isWhitespaceOrEmpty(str)) {
                        return -1;
                }

                const tuple<int,std::string> ns = toIntOrString(str);

                if (get<0>(ns)>=0 && (unsigned)get<0>(ns)<scene.cameraCount()) {
                        index = get<0>(ns);
                } else {
                        for (unsigned int i=0; i<scene.cameraCount(); ++i) {
                                if (scene.camera(i).title == get<1>(ns)) {
                                        index = i;
                                        break;
                                } else if (index < 0) {
                                        if (std::string::npos !=
                                                scene.camera(i).title.find(get<1>(ns)))
                                                index = i;
                                }
                        }
                }
                if (index < 0) {
                        std::cout << "Number or name \"" << str << "\" "
                                << "not found. Please type in a valid "
                                << "number or [partial] name." << std::endl;
                }
                return index;
        }
        void queryCamera (redshift_file::Scene &scene, const Options & options) {
                using namespace redshift_file;
                using namespace std;
                if (scene.cameraCount()>1) {
                        int index = -1;

                        // At first, try to match what was given as an option to us.
                        if (!isWhitespaceOrEmpty(options.useCamera)) {
                                index = queryCameraMatch (scene, options.useCamera);
                        }

                        // If still not, dump a menu.
                        if (index < 0) {
                                std::cout << "\nThere are multiple cameras present: \n\n";
                                for (unsigned int i=0; i<scene.cameraCount(); ++i) {
                                        const Camera &cam = scene.camera(i);
                                        std::cout << "  [" << i << "] "
                                        << cam.title
                                        << "\n";
                                }
                                std::cout << "\nWhich one do you want to use (number or [partial] name)? "<< std::endl;

                                while (index<0) {
                                        std::string str;
                                        std::getline (std::cin,str);
                                        index = queryCameraMatch (scene, str);
                                }
                        }

                        std::cout << "You have chosen [" << index << "], \""
                                << scene.camera(index).title << "\"." << std::endl;

                        Camera tmp = scene.camera(index);
                        scene.pruneCameras();
                        scene.addCamera (tmp);
                } else if (scene.cameraCount() == 0) {
                        std::cout << "There are no camera settings. Please consult the documentation."
                                << std::endl;
                        throw std::exception ();
                }
        }


        void renderSdl (
                redshift_file::Scene const &scened,
                const Options & options
        ) {
                using namespace redshift;
                using namespace redshift::camera;
                using namespace redshift::interaction;
                using namespace redshift::primitive;

                redshift::StopWatch stopWatch;


                const unsigned int
                        width = scened.renderSettings(0).width,
                        height = scened.renderSettings(0).height,
                        samplesPerPixel = scened.renderSettings(0).samplesPerPixel
                ;

                //RenderTarget::Ptr renderBuffer (new ColorRenderTarget(width,height));
                shared_ptr<Film> film (new Film (width, height));
                shared_ptr<Scene> scene = sceneDescriptionToScene(scened, film, 0, 0);


                RenderTarget::Ptr screenBuffer (new SdlRenderTarget(
                        width, height,
                        (options.doSaveOutput
                         && (options.saveFormat == Options::Bitmap
                            || options.saveFormat == Options::BitmapAndExr)
                        ) ? options.outputFile + ".bmp"
                          : "",
                        scened.filmSettings().colorscale,
                        scened.filmSettings().convertToSrgb
                ));

                UserCommandProcessor::Ptr commandProcessor (new SdlCommandProcessor());

                ProgressReporter::Ptr reporter (
                          new FilmToRenderTargetCopyingReporter(film, screenBuffer));

                scene->render(
                        reporter, commandProcessor,
                        samplesPerPixel,
                        scened.renderSettings(0).min_y,
                        scened.renderSettings(0).max_y,
                        scened.renderSettings(0).userSeed);
                //copy (renderBuffer, screenBuffer);
                //screenBuffer->flip();

                stopWatch.stop();
                std::stringstream ss;
                ss << "t:" << stopWatch();
                SDL_WM_SetCaption(ss.str().c_str(), ss.str().c_str());

                if (options.doSaveOutput
                   && (options.saveFormat == Options::Exr
                      || options.saveFormat == Options::BitmapAndExr)
                ) {
                        // it's a bit of legacy that at this point, the output filename
                        // may have a .bmp extension
                        saveOpenEXR(*film, (options.outputFile + ".exr").c_str());
                }
                if (options.pauseAfterRendering) {
                        while (!commandProcessor->userWantsToQuit())
                                commandProcessor->tick();
                }
        }
}

void read_and_render (Options const & options) {
        // TODO: make render settings an advice-thing, have multiple skies, have if-render-is member in sky (so that e.g. in "preview" there could be no ckouds)
        using namespace redshift_file;
        using namespace actuarius;
        using namespace std;
        Scene scene;

        if (0) {
                Object o;
                o.type = Object::lazy_quadtree;
                scene.addObject (o);
                o.type = Object::water_plane;
                scene.addObject (o);
                o.type = Object::triangle;
                scene.addObject (o);

                RenderSettings rs;
                rs.width = 800;
                rs.height = 600;
                rs.title = "preview-easy";
                scene.addRenderSettings (rs);
                rs.width = 800;
                rs.height = 600;
                rs.title = "preview-tough";
                scene.addRenderSettings (rs);
                rs.width = 3200;
                rs.height = 1600;
                rs.title = "full";
                rs.volumeIntegrator.type = VolumeIntegrator::single;
                scene.addRenderSettings (rs);

                Camera c;
                c.title = "hello-world";
                Transform t;
                t.type = Transform::move;
                t.x = 6; t.y = 7; t.z = 8;
                c.transforms.push_back (t);
                scene.addCamera (c);

                std::ofstream ofs("test.red");
                save_scene (scene, ofs);
        }

        {
                // Evolve read function from this
                std::ifstream ifs(options.inputFile.c_str());
                if (!ifs) {
                        std::cout << "Input file \""
                                << options.inputFile
                                << "\" could not be opened."
                                << std::endl;
                        return;
                }
                Scene scene;
                load_scene (scene, ifs);
                //IArchive (ifs) & pack("scene", scene);

                queryRenderSettings (scene, options);
                queryCamera (scene, options);
                renderSdl (scene, options);
        }
}

void read_angle_test() {
        enum UnitOfAngle {
                Degree, Radian
        };
        while(1) {
                std::string str;
                std::getline(std::cin,str);
                std::stringstream ss (str);
                double num;
                std::string unit;
                ss >> num;
                ss >> unit;

                std::clog << "<" << num << ">" << "<" << unit << ">\n";

                UnitOfAngle u;
                if (unit == "rad"
                || unit == "radian"
                || unit == "radians"
                || unit == "c"
                ) u = Radian;
                else if (unit == "deg"
                || unit == "degree"
                || unit == "degrees"
                || unit == "°"
                ) u = Radian;
        }
}

#undef main

int main (int argc, char *argv[]) {
        #if defined(_WIN32) || defined(_WIN64)
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
        #endif

        redshift::static_init();
        const optional<Options> oo = parseOptions(argc,argv);
        if (!oo)
                return 0;
        read_and_render(*oo);
        return 0;
}
