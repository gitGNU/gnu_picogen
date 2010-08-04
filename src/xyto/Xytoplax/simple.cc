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

#include <sstream>
#include <cmath>

#include "simple.hh"

#include <QGraphicsView>
#include <QMessageBox>

#include "ui_simple.h"

#include "../lsystem.hh"
#include "../xyto_ios.hh"



struct TurtleVector {
        double x,y,z;
        TurtleVector () : x(0), y(0), z(0) {}
        TurtleVector (double x, double y, double z) : x(x), y(y), z(z) {}

        TurtleVector& operator += (TurtleVector rhs) {
                x += rhs.x;
                y += rhs.y;
                z += rhs.z;
                return *this;
        }
};
double dot (TurtleVector lhs, TurtleVector rhs) {
        return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}
double length_sq (TurtleVector vec) {
        return dot(vec, vec);
}
double length (TurtleVector vec) {
        return std::sqrt(length_sq(vec));
}
TurtleVector normalize (TurtleVector vec) {
        const double len = 1/length(vec);
        return TurtleVector(vec.x*len,
                            vec.y*len,
                            vec.z*len);
}

TurtleVector cross (TurtleVector lhs, TurtleVector rhs) {
        return TurtleVector(
                lhs.y*rhs.z - lhs.z*rhs.y,
                lhs.z*rhs.x - lhs.x*rhs.z,
                lhs.x*rhs.y - lhs.y*rhs.x
        );
}
std::ostream& operator<< (std::ostream& o, TurtleVector const &rhs) {
        o << "[" << rhs.x << ", " << rhs.y << ", " << rhs.z << "]";
        return o;
}


class TurtleMatrix {
public:
        TurtleMatrix()  :
                m00(1), m01(0),  m02(0),
                m10(0), m11(1),  m12(0),
                m20(0), m21(0),  m22(1)
        {
        }

        TurtleMatrix(TurtleVector right, TurtleVector up, TurtleVector forw)
                :
                m00(right.x), m01(up.x),  m02(forw.x),
                m10(right.y), m11(up.y),  m12(forw.y),
                m20(right.z), m21(up.z),  m22(forw.z)
        {
        }

        TurtleMatrix (double m00, double m01, double m02,
                      double m10, double m11, double m12,
                      double m20, double m21, double m22
                      ) :
                m00(m00), m01(m01),  m02(m02),
                m10(m10), m11(m11),  m12(m12),
                m20(m20), m21(m21),  m22(m22)
        {
        }

        static TurtleMatrix X(double a) {
                using std::sin; using std::cos;
                return TurtleMatrix(
                        1, 0,      0,
                        0, cos(a), -sin(a),
                        0, sin(a), cos(a)
                );
        }
        static TurtleMatrix Y(double a) {
                using std::sin; using std::cos;
                return TurtleMatrix(
                        cos(a), 0, -sin(a),
                        0,      1, 0,
                        sin(a), 0, cos(a)
                );
        }
        static TurtleMatrix Z(double a) {
                using std::sin; using std::cos;
                return TurtleMatrix(
                        cos(a),  -sin(a), 0,
                        sin(a), cos(a), 0,
                        0,       0,      1
                );
        }

        TurtleMatrix operator * (TurtleMatrix rhs) const {
                TurtleMatrix ret;
                for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
                        ret[i+j*3] =
                                (*this)[i + 0*3] * rhs[0 + 3*j] +
                                (*this)[i + 1*3] * rhs[1 + 3*j] +
                                (*this)[i + 2*3] * rhs[2 + 3*j]
                        ;
                }
                return ret;
        }

        TurtleVector operator * (TurtleVector v) const {
                return TurtleVector(
                        m00*v.x + m01*v.y + m02*v.z,
                        m10*v.x + m11*v.y + m12*v.z,
                        m20*v.x + m21*v.y + m22*v.z
                );
        }

private:
        double& operator [] (unsigned int index) {
                switch (index) {
                default:
                case 0*3+0: return m00;
                case 0*3+1: return m01;
                case 0*3+2: return m02;
                case 1*3+0: return m10;
                case 1*3+1: return m11;
                case 1*3+2: return m12;
                case 2*3+0: return m20;
                case 2*3+1: return m21;
                case 2*3+2: return m22;
                }
        }
        double operator [] (unsigned int index) const {
                switch (index) {
                default:
                case 0*3+0: return m00;
                case 0*3+1: return m01;
                case 0*3+2: return m02;
                case 1*3+0: return m10;
                case 1*3+1: return m11;
                case 1*3+2: return m12;
                case 2*3+0: return m20;
                case 2*3+1: return m21;
                case 2*3+2: return m22;
                }
        }

        double m00, m01, m02;
        double m10, m11, m12;
        double m20, m21, m22;
};


/*
struct Turtle {
        float x, y, z;
        float theta, phi;

        Turtle() : x(0), y(0), z(0), theta(3.14159/2.), phi(3.14159/2.) {}

        void forward (float f) {
                using std::sin; using std::cos;
                x += f * sin(theta) * cos(phi);
                y += f * sin(theta) * sin(phi);
                z += f * cos(theta);
        }

        void turnLeft (float f=3.14159/2) {
                phi += f;
        }
        void turnRight (float f=3.14159/2) {
                phi -= f;
        }
};
*/
struct Turtle {
        TurtleVector position;
        TurtleMatrix rotation;

        Turtle() {
                pitchUp(3.14159*0.5);
        }

        void forward (float f) {
                position += rotation*TurtleVector(0,0,f);
        }

        void turnLeft (float f) {
                rotation = TurtleMatrix::Y(f) * rotation;
        }
        void turnRight (float f) {
                rotation = TurtleMatrix::Y(-f) * rotation;
        }

        void pitchUp (float f) {
                rotation = TurtleMatrix::X(-f) * rotation;
        }
        void pitchDown (float f) {
                rotation = TurtleMatrix::X(f) * rotation;
        }

        void rollLeft (float f) {
                std::cout << "before roll-left: " << up() << std::endl;
                rotation = TurtleMatrix::Z(f) * rotation;
                std::cout << "after roll-left: " << up() << std::endl;
        }
        void rollRight (float f) {
                rotation = TurtleMatrix::Z(-f) * rotation;
        }

        TurtleVector up() const { return rotation*TurtleVector(0,1,0); }
        TurtleVector right() const { return rotation*TurtleVector(1,0,0); }

        void rollToVertical() {
                const TurtleVector
                        up = TurtleVector (0,1,0),
                        forward = normalize(rotation*TurtleVector(0,0,1)),
                        newRight = normalize(cross(up,forward)),
                        newUp = normalize (cross(forward,newRight));

                std::cout << "right-before:" << this->right() << std::endl;
                rotation = TurtleMatrix(newRight, newUp, forward);
                std::cout << "right-after:" << this->right() << std::endl;
        }
};



boost::optional<LSystem> compile(const char*);

Simple::Simple(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Simple)
{
        ui->setupUi(this);

        /*Turtle a;
        a.rollLeft(0.0174532925 * 45);
        a.rollToVertical();

        exit(0);*/

        // Koch curve:
        //ui->sourceCode->setPlainText("foo: foo --> foo right foo left foo left foo right foo;\n"


        ui->sourceCode->setPlainText(

                        //"axiom: f(20) rollright(90) up(90)  f(20) up(135) f(20);"

                        /*
R=1.456;
a=120; // <-- tweak for fun
axiom:up(90) A(1);
p1: A(s) --> f(s)[right(a)A(s/R)][left(a)A(s/R)];
                         */

                        // abop p. 59
                        "r1=0.9;\n"
                        "r2=0.7;\n"
                        "a1=10;\n"
                        "a2=60;\n"
                        "wr=0.707;\n"
                        "\n"
                        "axiom: up(90) A(1,10);\n"
                        "p1: A(l,w) --> f(l) [down(a1)B(l*r1,w*wr)] rollright(180) [down(a2)B(l*r2,w*wr)];\n"
                        "p2: B(l,w) --> f(l) [left(a1) vert B(l*r1,w*wr)] [right(a2) vert B(l*r2,w*wr)];\n"
                        //*/

                        // abop p. 56
                        /*
                        "\n"
                        "r1=0.9;\n"
                        "r2=0.6;\n"
                        "a0=45;\n"
                        "a2=45;\n"
                        "d=137.5;\n"
                        "wr=0.707;\n"
                        "\n"
                        "axiom: A(1, 10);\n"
                        "\n"
                        "p1 : A(l,w) --> f(l) [down(a0)       B(l*r2, w*wr)] rollright(d) A(l*r1, w*wr);\n"
                        "p2 : B(l,w) --> f(l) [right(a2) vert C(l*r2, w*wr)] C(l*r1, w*wr);\n"
                        "p3 : C(l,w) --> f(l) [left(a2)  vert B(l*r2, w*wr)] B(l*r1, w*wr);\n"
                        "\n" //*/

                        /*"f: f --> f;\n"
                        "axiom: f(50) [left(90) f(10)]   left(45) up(70)  f(50); "*/

                        /*
                        "axiom: x(10);\n"
                        "f0:  x(x)  -->  f(x)   [left(75)  x(x*0.6)] right(7) f(x) [right(75) x(x*0.6)] x(0.5*x);\n"
                        */


                        // ABoP, p. 25, figure f

                        /*"axiom: x(10);\n"
                        "f0:  x(x)  -->  f(x) left(22.5) [[x(x)]right(22.5)x(x)]right(22.5)f(x)[right(22.5)f(x)x(x)]left(22.5)x(x);\n"
                        "f1: f(x) --> f(x)f(x);\n"
                        //*/


        );
}



Simple::~Simple() {
        delete ui;
}



void draw (Pattern pat, Turtle turtle, QGraphicsScene &scene) {
        typedef Pattern::const_iterator It;

        for (It it = pat.begin(); it!=pat.end(); ++it) {
                Segment seg = *it;
                if (seg.type() == Segment::Branch) {
                        draw (seg.branch(), turtle, scene);
                } else if (seg.type() == Segment::Letter) {
                        if (seg.name() == "left") {
                                if (!seg.parameterList().empty()) {
                                        turtle.turnLeft(seg.parameterList()[0].toReal() * 0.0174532925);
                                } else {
                                        turtle.turnLeft(0.5);
                                }
                        } else if (seg.name() == "right") {
                                if (!seg.parameterList().empty())
                                        turtle.turnRight(seg.parameterList()[0].toReal() * 0.0174532925);
                                else
                                        turtle.turnRight(0.5);
                        } else if (seg.name() == "up") {
                                if (!seg.parameterList().empty()) {
                                        turtle.pitchUp(seg.parameterList()[0].toReal() * 0.0174532925);
                                } else {
                                        turtle.pitchUp(0.5);
                                }
                        } else if (seg.name() == "down") {
                                if (!seg.parameterList().empty())
                                        turtle.pitchDown(seg.parameterList()[0].toReal() * 0.0174532925);
                                else
                                        turtle.pitchDown(0.5);
                        } else if (seg.name() == "rollleft") {
                                if (!seg.parameterList().empty()) {
                                        turtle.rollLeft(seg.parameterList()[0].toReal() * 0.0174532925);
                                } else {
                                        turtle.rollLeft(0.5);
                                }
                        } else if (seg.name() == "rollright") {
                                if (!seg.parameterList().empty())
                                        turtle.rollRight(seg.parameterList()[0].toReal() * 0.0174532925);
                                else
                                        turtle.rollRight(0.5);
                        } else if (seg.name() == "vert") {
                                turtle.rollToVertical();
                        } else if (seg.name() == "f"){
                                const Turtle oldBoy = turtle;

                                if (!seg.parameterList().empty())
                                        turtle.forward(seg.parameterList()[0].toReal());
                                else
                                        turtle.forward(1);

                                scene.addLine(oldBoy.position.x, -oldBoy.position.y,
                                              turtle.position.x, -turtle.position.y);
                        }
                }
        }
}




void Simple::on_draw_clicked() {
        const boost::optional<LSystem> lsys =
                compile(ui->sourceCode->toPlainText().toAscii());
        if (!lsys) {
                ui->outputPattern->setPlainText("<invalid L-System!>");
                return;
        }

        //--
        QGraphicsScene *scene = new QGraphicsScene (this);
        scene->addEllipse(-1,-1,2,2);

        ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
        draw (lsys->run(ui->numIterations->value()), Turtle(), *scene);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void Simple::on_write_clicked() {
        const boost::optional<LSystem> lsys =
                compile(ui->sourceCode->toPlainText().toAscii());
        if (!lsys) {
                ui->outputPattern->setPlainText("<invalid L-System!>");
                return;
        }
        std::stringstream ss;
        for (int i=0; i<ui->numIterations->value(); ++i) {
                const Pattern pat = lsys->run(i);
                ss << pat << '\n';
        }
        ui->outputPattern->setPlainText(QString::fromStdString(ss.str()));
}



void Simple::resizeEvent(QResizeEvent *) {
        ui->graphicsView->fitInView(ui->graphicsView->sceneRect(),
                                    Qt::KeepAspectRatio);
}
