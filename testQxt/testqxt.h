#ifndef TESTQXT_H
#define TESTQXT_H

#include <QtGui/QMainWindow>
#include "ui_testqxt.h"

class testQxt : public QMainWindow
{
	Q_OBJECT

public:
	testQxt(QWidget *parent = 0, Qt::WFlags flags = 0);
	~testQxt();

private:
	Ui::testQxtClass ui;
};

#endif // TESTQXT_H
