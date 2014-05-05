#include "tests.h"

Tests::Tests(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{

	ui.setupUi(this);

	ui.gradientWidget->setGradient(&grad);
	ui.colourMarkers->setOrientation(Qt::Horizontal);
	ui.colourMarkers->setSpan(20,60);
	ui.colourMarkers->setLowerPosition(24);
}

Tests::~Tests()
{

}
