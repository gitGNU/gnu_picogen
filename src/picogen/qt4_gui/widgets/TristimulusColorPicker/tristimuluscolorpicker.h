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

#ifndef TRISTIMULUSCOLORPICKER_HH
#define TRISTIMULUSCOLORPICKER_HH

#include <QWidget>
#include <QColor>

namespace Ui {
    class TristimulusColorPicker;
}

namespace picogen { namespace qt4_gui {

class TristimulusColor {
        double r, g, b;
public:

        TristimulusColor () : r(0), g(0), b(0) {}

        explicit TristimulusColor (QColor const &col)
                : r(col.redF())
                , g(col.greenF())
                , b(col.blueF())
        {}

        static TristimulusColor fromRgb (int r, int g, int b) {
                TristimulusColor ret;
                ret.r = r / 255.;
                ret.g = g / 255.;
                ret.b = b / 255.;
                return  ret;
        }

        static TristimulusColor fromRgbf (double r, double g, double b) {
                TristimulusColor ret;
                ret.r = r;
                ret.g = g;
                ret.b = b;
                return  ret;
        }

        void setRed (int v)   { r = v / 255.; }
        void setGreen (int v) { g = v / 255.; }
        void setBlue (int v)  { b = v / 255.; }

        void setRedF (double v)   { r = v; }
        void setGreenF (double v) { g = v; }
        void setBlueF (double v)  { b = v; }

        int red()   const { return r * 255; }
        int green() const { return g * 255; }
        int blue()  const { return b * 255; }

        double redF()   const { return r; }
        double greenF() const { return g; }
        double blueF()  const { return b; }

        bool isGray(double epsilon = 0.0001) const {
                return r >= g-epsilon && r <= g+epsilon
                    && r >= b-epsilon && r <= b+epsilon
                ;
        }

        void makeConvertibleToQColor (
                TristimulusColor &color, double &min, double &max
        ) const {
                min = r;
                min = g<min ? g : min;
                min = b<min ? b : min;

                max = r;
                max = g>max ? g : max;
                max = b>max ? b : max;

                if (min>=0 && max<=1) {
                        min = 0;
                        max = 1;
                        color = *this;
                        return;
                }

                if (min > 0)
                       min = 0;
                if (max < 0)
                       max = 0;

                const double range = max - min;
                color.r = (r - min) / range;
                color.g = (g - min) / range;
                color.b = (b - min) / range;
        }

        QColor toQColor (double min=0, double max=1) const {
                const double range = max - min;
                return QColor::fromRgbF((r-min)/range,
                                        (g-min)/range,
                                        (b-min)/range);
        }


        bool operator == (TristimulusColor const &rhs) const {
                return r == rhs.r
                    && g == rhs.g
                    && b == rhs.b
                    ;
        }
        bool operator != (TristimulusColor const &rhs) const {
                return !(*this == rhs);
        }
};



class TristimulusColorPicker : public QWidget {
        Q_OBJECT
public:
        TristimulusColorPicker(QWidget *parent = 0);
        ~TristimulusColorPicker();

        TristimulusColor color() const ;
        void setColor (TristimulusColor const &TristimulusColor) ;

protected:
        void changeEvent(QEvent *e);
        void resizeEvent(QResizeEvent *);

private:
        void drawColorPreview();
        Ui::TristimulusColorPicker *ui;
        bool isUpdating;
        TristimulusColor color_;
        double min, max, range;

        void setColor_ (TristimulusColor const &TristimulusColor) ;


//------------------------------------------------------------------------------
// Signals + Slots
//------------------------------------------------------------------------------
signals:
        void colorChanged (const TristimulusColor & color);

public slots:
        void on_triangle_colorChanged(const QColor & color);

private slots:
        void on_spinMax_valueChanged(double );
        void on_spinMin_valueChanged(double );

        void on_spinR_valueChanged(double);
        void on_spinG_valueChanged(double);
        void on_spinB_valueChanged(double);

        void on_spinH_valueChanged(double);
        void on_spinS_valueChanged(double);
        void on_spinV_valueChanged(double);

        void on_spinC_valueChanged(double);
        void on_spinM_valueChanged(double);
        void on_spinY_valueChanged(double);
        void on_spinK_valueChanged(double);

        void on_showRGB_toggled(bool checked);
        void on_showHSV_toggled(bool checked);
        void on_showCMYK_toggled(bool checked);
};

} }

#endif // TRISTIMULUSCOLORPICKER_HH
