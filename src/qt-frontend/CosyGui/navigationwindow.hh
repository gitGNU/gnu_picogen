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

#ifndef NAVIGATIONWINDOW_HH
#define NAVIGATIONWINDOW_HH

#include <QWidget>
#include "redshift/include/smart_ptr.hh"

namespace Ui {
        class NavigationWindow;
}

namespace cosyscene {
        class Navigation;
        class Scene;
}

class NavigationWindow : public QWidget {
        Q_OBJECT

public:
        explicit NavigationWindow(QWidget *parent = 0);
        ~NavigationWindow();

        void setNavigation (redshift::shared_ptr<cosyscene::Navigation>,
                            bool blockSignals=true);

signals:
        void navigationChanged ();

public slots:
        void sceneInvalidated(redshift::shared_ptr<cosyscene::Scene> scene);

private:
        Ui::NavigationWindow *ui;
        redshift::shared_ptr<cosyscene::Navigation> navigation_;

        void updateViews ();
        void updateFromViews();

private slots:
        void on_zSpin_valueChanged(double );
        void on_ySpin_valueChanged(double );
        void on_xSpin_valueChanged(double );
        void on_refreshButton_clicked();

        void on_rollSpin_valueChanged(double);
        void on_rollDial_valueChanged(int);

        void on_pitchSpin_valueChanged(double );
        void on_pitchSlider_valueChanged(int);

        void on_yawSpin_valueChanged(double);
        void on_yawDial_sliderMoved(int);

        void on_showRenderTab_clicked();
        void on_showRealTimeTab_clicked();
        void on_showPrecisionTab_clicked();
};

#endif // NAVIGATIONWINDOW_HH