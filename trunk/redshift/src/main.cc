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

#include <SDL/SDL.h>
#include <omp.h>

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>

#ifdef AMALGAM
#include "../../include/redshift.hh"
#include "../../include/rendertargets/sdlrendertarget.hh"
#include "../../include/rendertargets/colorrendertarget.hh"
#include "../../include/cameras/pinhole.hh"
#include "../../include/interaction/sdlcommandprocessor.hh"
#include "../../include/basictypes/height-function.hh"
#else
#include "../include/redshift.hh"
#include "../include/rendertargets/sdlrendertarget.hh"
#include "../include/rendertargets/colorrendertarget.hh"
#include "../include/cameras/pinhole.hh"
#include "../include/interaction/sdlcommandprocessor.hh"
#include "../include/basictypes/height-function.hh"
#endif

#include "../../actuarius/actuarius.hh"



namespace redshift { namespace interaction {
class RenderTargetCopyingReporter : public ProgressReporter {
public:

        RenderTargetCopyingReporter (
                RenderTarget::Ptr src,
                RenderTarget::Ptr target_
        )
        : source (src), target (target_)
        {}



        void report (RenderTarget::ReadLockPtr sourcel,
                                              int completed, int total) const {
                if (lastTime() < 0.5)
                        return;
                lastTime.restart();

                real_t const finished = static_cast<real_t>(completed)
                                          / static_cast<real_t>(total);
                /*
                if (total>0.0) {
                        cout << real_t(100)*(finished) << "%"
                             << endl;
                } else {
                        cout << completed << endl;
                }
                */
                copy (source, sourcel, target);
                target->flip();
        }



        void reportDone () const {
                redshift::copy (source, target);
                target->flip();
        }

private:
        RenderTargetCopyingReporter();
        RenderTargetCopyingReporter(RenderTargetCopyingReporter const &);
        RenderTargetCopyingReporter &
                        operator = (RenderTargetCopyingReporter const &);

        RenderTarget::ConstPtr source;
        RenderTarget::Ptr target;

        mutable StopWatch lastTime;
};

} }


#ifdef AMALGAM
#include "../../../quatsch/quatsch.hh"
#include "../../../quatsch/frontend/jux.hh"
#include "../../../quatsch/backend/est/backend.hh"
#else
#include "../../quatsch/quatsch.hh"
#include "../../quatsch/frontend/jux.hh"
#include "../../quatsch/backend/est/backend.hh"
#endif
#include "../../quatsch/configurable-functions/noise2ddef.hh"
#include "../../quatsch/configurable-functions/layerednoise2ddef.hh"


namespace redshift {
class QuatschHeightFunction : public redshift::HeightFunction {
private:
        // quatsch
        typedef quatsch::backend::est::Backend <redshift::real_t, const redshift::real_t *> backend_t;
        typedef backend_t::Function Function;
        typedef backend_t::FunctionPtr FunctionPtr;
        typedef backend_t::scalar_t scalar_t;
        typedef backend_t::parameters_t parameters_t;
        typedef quatsch::frontend::jux::Compiler <backend_t> Compiler;
        typedef Compiler::ConfigurableFunctionsMap FunctionSet;

        static Compiler::ConfigurableFunctionsMap addfuns() {
                using namespace redshift;


                quatsch::ICreateConfigurableFunction<Function>::ConfigurableFunctionDescriptionPtr noiseDesc (
                        new quatsch::CreateConfigurableFunction <
                                quatsch :: configurable_functions :: Noise2d <Function, Compiler>,
                                Function
                        >
                );
                quatsch::ICreateConfigurableFunction<Function>::ConfigurableFunctionDescriptionPtr layeredNoise2dDesc (
                        new quatsch::CreateConfigurableFunction <
                                quatsch :: configurable_functions :: LayeredNoise2d <Function, Compiler>,
                                Function
                        >
                );

                Compiler::ConfigurableFunctionsMap addfuns;
                addfuns.addSymbol ("Noise2d", noiseDesc);
                addfuns.addSymbol ("LayeredNoise2d", layeredNoise2dDesc);
                return addfuns;
        }

        FunctionSet functionSet;
        Compiler::FunctionPtr fun;
        std::stringstream errors;

public:
        real_t operator ()
         (real_t const & u, real_t const & v) const {
                //real_t const d = sqrt (u*u + v*v);
                const real_t p [] = { u, v };
                return (*fun) (p);
        }


        QuatschHeightFunction (const std::string code)
        : functionSet(addfuns())
        , fun (Compiler::compile (
                "x;y",
                //"(* 0.5 (* (sin (* x 2.0)) (sin(* y 2.0)) ))",
                /*"(- 1 (abs ([LayeredNoise2d filter{cosine} seed{12} frequency{0.25} layercount{4} persistence{0.54} levelEvaluationFunction{(abs h)}] "
                "  (+ y (^ (abs ([LayeredNoise2d filter{cosine} seed{12} frequency{0.25} layercount{12} persistence{0.54} levelEvaluationFunction{(abs h)}] x y)) 4))"
                "  (+ y (^ (abs ([LayeredNoise2d filter{cosine} seed{12} frequency{0.25} layercount{12} persistence{0.54} levelEvaluationFunction{(abs h)}] x y)) 4))"
                ")))",*/
                code,
                //"(* x 0.1)",
                functionSet,
                errors))
        {
        }
};
} // namespace redshift


class HeightFunction : public redshift::HeightFunction {
        typedef redshift::real_t real_t;
        typedef redshift::fixed_point_t fixed_point_t;
        real_t operator ()
         (real_t const & u, real_t const & v) const {
                real_t const d = sqrt (u*u + v*v);
                return (sin(u*0.7)*sin(v*0.7)) - 4;
        }
};



struct Object {
        enum Type {
                lazy_quadtree,
                horizon_plane
        };

        Type type;



        // - Object data ------------------------------------------------------
        // LazyQuadtree.
        std::string lazyQuadtreeCode;
        unsigned int lazyQuadtreeMaxRecursion;
        float lazyQuadtreeLodFactor;
        float lazyQuadtreeSize;
        // --------------------------------------------------------------------



        // - Serialization ----------------------------------------------------
        template<typename Arch>
        void serialize (Arch &arch) {
                using actuarius::pack;
                arch & pack ("type", typenames, type);

                switch (type) {
                case lazy_quadtree:
                        arch & pack ("code", lazyQuadtreeCode);
                        arch & pack ("max-recursion", lazyQuadtreeMaxRecursion);
                        arch & pack ("lod-factor", lazyQuadtreeLodFactor);
                        arch & pack ("size", lazyQuadtreeSize);
                        break;
                case horizon_plane: break;
                };
        }
        // --------------------------------------------------------------------



        // - Ctor / Assignment ------------------------------------------------
        Object ()
        : type(lazy_quadtree)
        , lazyQuadtreeCode("(sin x)")
        , lazyQuadtreeMaxRecursion(6)
        , lazyQuadtreeLodFactor(0.0025f)
        , lazyQuadtreeSize(10000.f)
        {}

        Object (Object const & rhs)
        : type (rhs.type)
        , lazyQuadtreeCode         (rhs.lazyQuadtreeCode)
        , lazyQuadtreeMaxRecursion (rhs.lazyQuadtreeMaxRecursion)
        , lazyQuadtreeLodFactor    (rhs.lazyQuadtreeLodFactor)
        , lazyQuadtreeSize         (rhs.lazyQuadtreeSize)
        {}

        Object const & operator = (Object const & rhs) {
                type = rhs.type;
                lazyQuadtreeCode         = rhs.lazyQuadtreeCode;
                lazyQuadtreeMaxRecursion = rhs.lazyQuadtreeMaxRecursion;
                lazyQuadtreeLodFactor    = rhs.lazyQuadtreeLodFactor;
                lazyQuadtreeSize         = rhs.lazyQuadtreeSize;
                return *this;
        }
        // --------------------------------------------------------------------



        // - Static data ------------------------------------------------------
        static const actuarius::Enum<Type> typenames;
        // --------------------------------------------------------------------
};
const actuarius::Enum<Object::Type> Object::typenames
         ( actuarius::Nvp<Object::Type>(Object::lazy_quadtree, "lazy_quadtree")
         | actuarius::Nvp<Object::Type>(Object::horizon_plane, "horizon_plane")
         /*| Nvp<Foo>(three, "three")
         | Nvp<Foo>(four, "four")*/
         );



struct Transform {
};


struct RedshiftJob {
        // Basic configuration.
        unsigned int width, height;
        unsigned int samplesPerPixel;

        // Description.
        std::string title;
        std::string description;

        // Objects.
        struct Objects {
                std::vector<Object> objects;

                // Serialization.
                template<typename Arch>
                void serialize (Arch &arch) {
                        using actuarius::pack;
                        arch & pack ("object", objects);
                }

                Objects () {}

                Object &operator [] (unsigned int i) {
                        return objects[i];
                }
                const Object &operator [] (unsigned int i) const {
                        return objects[i];
                }
                void push_back (Object const &ob) {
                        objects.push_back (ob);
                }
        };
        Objects objects;

        // Constructor.
        RedshiftJob ()
        : width(800), height(600), samplesPerPixel(1)
        , title("default-title")
        , description("default-description")
        , objects()
        {}

        // Serialization.
        template<typename Arch>
        void serialize (Arch &arch) {
                using actuarius::pack;
                arch & pack ("width", width);
                arch & pack ("height", height);
                arch & pack ("samples-per-pixel", samplesPerPixel);

                arch & pack ("title", title);
                arch & pack ("description", description);

                arch & pack ("objects", objects);
        }
};


struct Advice {
        struct Row {
                float x,y,z,w;
                template<typename Arch>
                void serialize (Arch &arch) {
                        using actuarius::pack;
                        arch & pack(x) & pack(y) & pack(z) & pack(w);
                }
        };

        enum Type {
                foo,
                bar
        };
        Type type;
        float tx, ty;
        std::string tz;
        Row row0;
        Row row1;
        Row row2;
        Row row3;

        // Serialization.
        template<typename Arch>
        void serialize (Arch &arch) {
                using actuarius::pack;
                //arch & pack ("num", num) & pack ("alpha", alpha);
                //arch & pack ("x",tx) & pack ("y",ty) & pack ("z",tz);
                arch & pack (row0);
                arch & pack (row1);
                arch & pack (row2);
                arch & pack (row3);
        }

        static const actuarius::Enum<Advice::Type> typenames;
};
const actuarius::Enum<Advice::Type> Advice::typenames
        ( actuarius::Nvp<Advice::Type>(Advice::foo, "foo")
        | actuarius::Nvp<Advice::Type>(Advice::bar, "bar")
        );


void doActuariusTest () {
        using namespace actuarius;
        std::ifstream os ("foo.txt");
        //std::ofstream os ("foo.txt");
        //std::ostream &os = std::cout;

        std::vector<Advice> advices;

        Advice a;
        /*a.type = Advice::foo; advices.push_back(a);
        a.type = Advice::bar; advices.push_back(a);*/

        IArchive(os) & pack ("frob", &Advice::type, Advice::typenames, advices);

        /*for (std::vector<Advice>::iterator it=advices.begin(); it!=advices.end(); ++it) {
                switch (it->type) {
                case Advice::foo: std::cout << "foo :), " << it->tx << ", " << it->ty << ", " << it->tz << "\n"; break;
                case Advice::bar: std::cout << "bar :D, " << it->tx << ", " << it->ty << ", " << it->tz << "\n"; break;
                };
        }*/
        Advice s = advices[0];
        std::cout << s.row0.x << ',' << s.row0.y << ',' << s.row0.z << ',' << s.row0.w << '\n';
        std::cout << s.row1.x << ',' << s.row1.y << ',' << s.row1.z << ',' << s.row1.w << '\n';
        std::cout << s.row2.x << ',' << s.row2.y << ',' << s.row2.z << ',' << s.row2.w << '\n';
        std::cout << s.row3.x << ',' << s.row3.y << ',' << s.row3.z << ',' << s.row3.w << '\n';

        /*if (1) {
                std::ofstream os ("in.txt");

                RedshiftJob job;
                //job.redshiftVersion = (0<<24)|(3);
                job.width = 1024;
                job.height = 768;
                job.samplesPerPixel = 3;

                job.title = "actuarius-test";
                job.description = "grind grind grind";

                Object ob;
                ob.type = Object::lazy_quadtree;
                ob.lazyQuadtreeCode = "(sin x)";
                job.objects.push_back (ob);

                OArchive(os) & pack ("redshift-job", job);

        }
        {
                std::ifstream is ("in.txt");
                RedshiftJob job;
                IArchive(is) & pack ("redshift-job", job);
                std::cout << job.width << "x" << job.height << "x" << job.samplesPerPixel << std::endl;
                std::cout << job.title << ":" << job.description << std::endl;
                std::cout << job.objects[0].lazyQuadtreeCode << std::endl;
        }*/
}




void run() {
        using namespace redshift;
        using namespace redshift::camera;
        using namespace redshift::interaction;
        using namespace redshift::primitive;


        redshift::StopWatch stopWatch;

        // TODO replace RenderTarget with Film?
        //    i mean, a "RenderTarget" might be flipable, but a Film not, or so
        int const width = 1680/6;
        int const height = width;
        int const AA = 1;
        RenderTarget::Ptr renderBuffer (new ColorRenderTarget(width,height));
        shared_ptr<Camera> camera (new Pinhole(
                renderBuffer, 1.0f,
                redshift::Transform::translation (0,5000,0)
                *redshift::Transform::rotationX(constants::pi*0.5f)
        ));

        shared_ptr<redshift::HeightFunction> heightFunction;
        shared_ptr<redshift::HeightFunction> distortHeightFunction;
        try {
                heightFunction = shared_ptr<redshift::HeightFunction> (
                        new ::redshift::QuatschHeightFunction(
"(+"
  "(* 2000 ([LayeredNoise2d filter{cosine} seed{57} frequency{0.001} layercount{3} persistence{0.4}] x y))"
"  (* 100 ([LayeredNoise2d filter{cosine} seed{542} frequency{0.01} layercount{9} persistence{0.53}] x y))"
") "

//"(+ -1100 (* 2200 (- 1 (abs ([LayeredNoise2d filter{cosine} seed{4} frequency{0.00025} layercount{8} persistence{0.5} levelEvaluationFunction{(abs h)}] (+ 100000 x) (+ 100000 y))))))"
//                "(* 3 (sin (* 0.01 x)) (sin (* 0.01 y)))"
                ));
                distortHeightFunction = shared_ptr<redshift::HeightFunction> (
                        new ::redshift::QuatschHeightFunction(
                          "(* 0.05 ([LayeredNoise2d filter{cosine} seed{13} frequency{0.02} layercount{10} persistence{0.63}] x y))"
                ));
                /*for (int i=0; i<50; ++i) {
                        std::cout << (*distortHeightFunction)(rand()/(RAND_MAX+1.f),rand()/(RAND_MAX+1.f)) << std::endl;
                }*/
        } catch (...) { // TODO (!!!)
        }

        primitive::List *list = new List;
        /*
        list->add (shared_ptr<primitive::Primitive> (new ClosedSphere (vector_cast<Point>(PointF(0,15,-5420)), 10)));
        list->add (shared_ptr<primitive::Primitive> (new ClosedSphere (vector_cast<Point>(PointF(0,35,-5420)), 10)));
        list->add (shared_ptr<primitive::Primitive> (new ClosedSphere (vector_cast<Point>(PointF(0,55,-5420)), 10)));
        list->add (shared_ptr<primitive::Primitive> (new ClosedSphere (vector_cast<Point>(PointF(0,75,-5420)), 10)));*/
        //list->add (shared_ptr<primitive::Primitive> (new ClosedSphere (vector_cast<Point>(PointF(610,5,-3850)), 10)));
        list->add (shared_ptr<primitive::Primitive> (new LazyQuadtree (heightFunction, 10000, 4, 0.0025)));
        list->add (shared_ptr<primitive::Primitive> (new HorizonPlane (0, distortHeightFunction)));
        shared_ptr<primitive::Primitive> agg (list);

        shared_ptr<background::Preetham> preetham (new background::Preetham());
        preetham->setSunDirection(Vector(-7.0,1.001,1.001));
        preetham->setTurbidity(2.0f);
        preetham->setSunColor(redshift::Color(1.1,1,0.9)*70);
        preetham->setColorFilter(redshift::Color(1.0,1.0,1.0)*0.025);
        preetham->enableFogHack (false, 0.00025f, 150000);
        preetham->invalidate();

        /*
                shared_ptr<VolumeRegion> (new volume::Homogeneous (
                        Color::fromRgb(0.0001,0.00011,0.00012)*2.5, // absorption
                        Color::fromRgb(0.00015,0.00015,0.00015), // out scattering probability
                        Color::fromRgb(0.0002,0.00018,0.00015), // emission
                        0.1,// Henyey Greenstein
        */

        Scene Scene (
                renderBuffer,
                camera,
                agg,
                shared_ptr<Background> (new backgrounds::PreethamAdapter (preetham)),
                //shared_ptr<Background>(new backgrounds::Monochrome(Color::fromRgb(1,1,1)))
                //shared_ptr<Background>(new backgrounds::VisualiseDirection())
                shared_ptr<Integrator> (new DirectLighting(20/*ambient samples*/)),
                shared_ptr<VolumeRegion> (new volume::Exponential (
                        Color::fromRgb(1,1,0.8)*0.00025, // absorption
                        Color::fromRgb(1,1,1)*0.00025, // out scattering probability
                        Color::fromRgb(1,1,1)*0.0001, // emission
                        0.0 // Henyey Greenstein
                        , 1.f, 0.0075f, Point(0.f,0.f,0.f)
                )),//*/,
                shared_ptr<VolumeIntegrator> (new SingleScattering(50.f))
        );

        RenderTarget::Ptr screenBuffer (new SdlRenderTarget(width,height));

        UserCommandProcessor::Ptr commandProcessor (new SdlCommandProcessor());

        ProgressReporter::Ptr reporter (
                  new RenderTargetCopyingReporter(renderBuffer, screenBuffer));

        Scene.render(reporter, commandProcessor,AA);
        copy (renderBuffer, screenBuffer);
        screenBuffer->flip();

        stopWatch.stop();
        std::stringstream ss;
        ss << "t:" << stopWatch();
        SDL_WM_SetCaption(ss.str().c_str(), "picogen:redshift");

        while (!commandProcessor->userWantsToQuit())
                commandProcessor->tick();
}

#ifdef PICOGENLIB
int picogen_main ()
#else
int main (int, char*[])
#endif
{
        using namespace redshift;

        //doActuariusTest ();
        //return 0;

        try {
                // Initialize SDL video.
                if (SDL_Init (SDL_INIT_VIDEO) < 0) {
                        std::stringstream ss;
                        ss <<  "Unable to init SDL:\n" << SDL_GetError();
                        throw std::runtime_error (ss.str());
                }
                atexit(SDL_Quit);

                run();

        } catch (std::runtime_error &ex) {
                std::cerr << "Caught runtime error: "
                          << ex.what()
                          << std::endl;
        } catch (std::exception &ex) {
                std::cerr << "Caught general exception: "
                          << ex.what()
                          << std::endl;
        }
        return 0;
}
