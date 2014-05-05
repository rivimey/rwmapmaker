#ifndef TESTS_H
#define TESTS_H

#include <QtWidgets/QMainWindow>
#include "ui_tests.h"

#include "qxtgradient.h"

class Tests : public QMainWindow
{
	Q_OBJECT

	QxtGradient grad;

public:
	Tests(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Tests();

private:
	Ui::TestsClass ui;
};

#endif // TESTS_H
