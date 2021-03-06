//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2011  Sebastian Mach (*1983)
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

#ifndef LAZYQUADTREELODPLOT_HH
#define LAZYQUADTREELODPLOT_HH

#include <QWidget>

namespace Ui {
        class LazyQuadtreeLodPlot;
}
class QPainter;
class QRectF;

namespace picogen { namespace qt4_gui {

class LazyQuadtreeLodPlot : public QWidget {
        Q_OBJECT

public:
        explicit LazyQuadtreeLodPlot(QWidget *parent = 0);
        ~LazyQuadtreeLodPlot();

        bool eventFilter(QObject *, QEvent *);

private slots:
        void on_maxRecursion_valueChanged(int );
        void on_lodFactor_valueChanged(double );

private:
        Ui::LazyQuadtreeLodPlot *ui;
};

} }

#endif // LAZYQUADTREELODPLOT_HH
