
#include <QtGui/QPen>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QApplication>

#include "qxtgradientslider.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	QxtGradient grad;
	grad.clear();
	grad.append(new QxtGradientSegment(0.0, QColor(Qt::darkBlue), 0.25, 0.4, QColor(Qt::lightGray)));
	grad.append(new QxtGradientSegment(0.5, QColor(Qt::green),    0.75, 1.0, QColor(Qt::red)));
    QxtGradientSlider slider;
	slider.setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
	slider.setLineWidth(2);
	slider.setMinimumHeight(20);
	slider.setGradient(&grad);
    slider.show();
    return app.exec();
}
