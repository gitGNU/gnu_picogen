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

#ifndef PARAMETER_HH
#define PARAMETER_HH

#include <QtGui/QWidget>

namespace Ui {
    class Parameter;
}


class NodeItem;

class ParameterUi : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(ParameterUi)
public:
    explicit ParameterUi(NodeItem *node, QWidget *parent = 0);
    virtual ~ParameterUi();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Parameter *m_ui;
    NodeItem *node;

private slots:
    void on_comboBox_currentIndexChanged(int index);
};

#endif // PARAMETER_HH
