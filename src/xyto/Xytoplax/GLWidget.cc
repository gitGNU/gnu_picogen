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

#include "GLWidget.hh"
#include "../turtle.hh"
#include "../draw.hh"


class GLDisplayListMesh {
public:
        GLDisplayListMesh () {
                displayList = glGenLists(1);
                glNewList(displayList, GL_COMPILE);

                glColor3f(0, 1, 0);
                glLineWidth(2);
                glBegin(GL_LINES);
        }

        ~GLDisplayListMesh () {
                glEnd();
                glEndList();
        }

        void moveTo (Turtle state) {
                this->state = state;
        }

        void drawTo (Turtle newState) {
                glVertex3f(state.position.x,
                           state.position.y,
                           state.position.z);
                state = newState;
                glVertex3f(state.position.x,
                           state.position.y,
                           state.position.z);
        }

        GLuint result() const {
                return displayList;
        }
private:
        Turtle state;
        GLuint displayList;
};





GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
        xRot = 0;
        yRot = 0;
        zRot = 0;

        qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
        qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

        displayList = 0;
}


GLWidget::~GLWidget()
{
        makeCurrent();
        if (displayList != 0)
                glDeleteLists(displayList, 1);
}


QSize GLWidget::minimumSizeHint() const {
        return QSize(50, 50);
}


QSize GLWidget::sizeHint() const {
        return QSize(400, 400);
}
 /*void GLWidget::setXRotation(int angle)
  {
      qNormalizeAngle(angle);
      if (angle != xRot) {
          xRot = angle;
          emit xRotationChanged(angle);
          updateGL();
      }

  }*/


void GLWidget::initializeGL() {
        qglClearColor(qtPurple.dark());

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_MULTISAMPLE);
        static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}


void GLWidget::resizeGL(int width, int height) {
        int side = qMin(width, height);
        glViewport((width - side) / 2, (height - side) / 2, side, side);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        const double s = 1;
/*#ifdef QT_OPENGL_ES_1
        glOrthof(s*-0.5, s*+0.5, s*-0.5, s*+0.5, 0.0, 150.0);
#else
        glOrtho(s*-0.5, s*+0.5, s*-0.5, s*+0.5, 0.0, 150.0);
#endif*/
        gluPerspective(90*0.17, 1, 0, 100);
}


void GLWidget::paintGL() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0, 20.0, -7000.0);
        /*glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
        glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
        glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);*/
        glCallList(displayList);

        /*glBegin(GL_LINES);
        glVertex3f(-1,0,0);
        glVertex3f(1,0,0);
        glEnd();*/
}


void GLWidget::mousePressEvent(QMouseEvent *event) {
        lastPos = event->pos();
}


void GLWidget::mouseMoveEvent(QMouseEvent *event) {
        int dx = event->x() - lastPos.x();
        int dy = event->y() - lastPos.y();
        Q_UNUSED(dx)
        Q_UNUSED(dy)

        if (event->buttons() & Qt::LeftButton) {
                //setXRotation(xRot + 8 * dy);
                //setYRotation(yRot + 8 * dx);
        } else if (event->buttons() & Qt::RightButton) {
                //setXRotation(xRot + 8 * dy);
                //setZRotation(zRot + 8 * dx);
        }
        lastPos = event->pos();
}


void GLWidget::updateData(Pattern const &pat) {
        if (displayList != 0)
                glDeleteLists(displayList, 1);

        if (1) {
                {
                        GLDisplayListMesh dlm;
                        draw (pat, Turtle(), dlm);
                        displayList = dlm.result();
                }
                repaint();
        } else {
                displayList = glGenLists(1);
                glNewList(displayList, GL_COMPILE);

                glBegin(GL_LINES);
                glColor3f(0.5,0.7,0.9);
                glVertex3f(0,0,0);
                glVertex3f(10,50,0);
                glEnd();

                glEndList();

                repaint();
        }
}