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

#ifndef TURTLE_HH_INCLUDED_20100805
#define TURTLE_HH_INCLUDED_20100805

#include "turtlematrix.hh"

struct Turtle {
        TurtleVector position;
        TurtleMatrix rotation;
        TurtleVector tropism;
        double e;
        double diameter;
        double scale;

        Turtle() {
                pitchUp(3.14159*0.5);
                diameter = 1;
                tropism = TurtleVector(0,-1,0);
                e = 0.22;
                scale = 1;
        }


        void adjust (TurtleVector vector, double amount) {
                TurtleVector torque = cross (normalize(vector), heading_(1));
                rotation = rotation * TurtleMatrix::Rotate(
                                -amount*length(torque),
                                normalize(torque))
                           ;
        }

        void tropism_ (double distance, TurtleVector vector, double strength) {
                adjust (vector, strength);
                position += heading_(distance); //forward (distance);
        }


        void forward (double f) {
                tropism_ (f, tropism, e);
        }

        void turnLeft (double f) {
                rotation = TurtleMatrix::RotateY(f) * rotation;
        }
        void turnRight (double f) {
                rotation = TurtleMatrix::RotateY(-f) * rotation;
        }

        void pitchUp (double f) {
                rotation = TurtleMatrix::RotateX(-f) * rotation;
        }
        void pitchDown (double f) {
                rotation = TurtleMatrix::RotateX(f) * rotation;
        }

        void rollLeft (double f) {
                rotation = TurtleMatrix::RotateZ(f) * rotation;
        }
        void rollRight (double f) {
                rotation = TurtleMatrix::RotateZ(-f) * rotation;
        }

        void decrementDiameter (double f) {
                diameter = f;
        }

        void rollToVertical() {
                const TurtleVector
                        up = TurtleVector (0,1,0),
                        forward = normalize(rotation*TurtleVector(0,0,1)),
                        newRight = normalize(cross(up,forward)),
                        newUp = normalize (cross(forward,newRight));

                rotation = TurtleMatrix(newRight, newUp, forward);
        }

        TurtleVector disk (double phi) {
                const double radius = diameter * 0.5;
                return position + normal(phi) * radius;
        }
        TurtleVector normal (double phi) {
                return normalize( up_() * sin(phi)
                                + right_() * cos(phi));
        }

private:
        TurtleVector up_() const { return rotation*TurtleVector(0,1,0); }
        TurtleVector right_() const { return rotation*TurtleVector(1,0,0); }
        TurtleVector heading_(double f=1) const {
                return rotation*TurtleVector(0,0,f);
        }

};

#endif // TURTLE_HH_INCLUDED_20100805
