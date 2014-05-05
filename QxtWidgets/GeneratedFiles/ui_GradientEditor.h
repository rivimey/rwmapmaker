/********************************************************************************
** Form generated from reading UI file 'GradientEditor.ui'
**
** Created by: Qt User Interface Compiler version 5.0.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GRADIENTEDITOR_H
#define UI_GRADIENTEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qxtspanslider.h"

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QWidget *routeMarkers;
    QWidget *gradient;
    QxtSpanSlider *colourMarkers;
    QHBoxLayout *horizontalLayout;
    QPushButton *loadGGR;
    QSpacerItem *horizontalSpacer;
    QPushButton *newSplit;
    QPushButton *removeSplit;
    QDialogButtonBox *buttonBox;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName(QStringLiteral("Form"));
        Form->resize(791, 236);
        verticalLayoutWidget = new QWidget(Form);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 791, 231));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(8);
        verticalLayout->setContentsMargins(8, 8, 8, 8);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        routeMarkers = new QWidget(verticalLayoutWidget);
        routeMarkers->setObjectName(QStringLiteral("routeMarkers"));

        verticalLayout->addWidget(routeMarkers);

        gradient = new QWidget(verticalLayoutWidget);
        gradient->setObjectName(QStringLiteral("gradient"));

        verticalLayout->addWidget(gradient);

        colourMarkers = new QxtSpanSlider(verticalLayoutWidget);
        colourMarkers->setObjectName(QStringLiteral("colourMarkers"));

        verticalLayout->addWidget(colourMarkers);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 6, -1, 6);
        loadGGR = new QPushButton(verticalLayoutWidget);
        loadGGR->setObjectName(QStringLiteral("loadGGR"));

        horizontalLayout->addWidget(loadGGR);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        newSplit = new QPushButton(verticalLayoutWidget);
        newSplit->setObjectName(QStringLiteral("newSplit"));
        newSplit->setMinimumSize(QSize(100, 0));

        horizontalLayout->addWidget(newSplit);

        removeSplit = new QPushButton(verticalLayoutWidget);
        removeSplit->setObjectName(QStringLiteral("removeSplit"));
        removeSplit->setMinimumSize(QSize(100, 0));

        horizontalLayout->addWidget(removeSplit);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(verticalLayoutWidget);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok|QDialogButtonBox::RestoreDefaults);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(Form);

        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QApplication::translate("Form", "Form", 0));
        loadGGR->setText(QApplication::translate("Form", "Load GGR File", 0));
        newSplit->setText(QApplication::translate("Form", "New Gradient Split", 0));
        removeSplit->setText(QApplication::translate("Form", "Remove Split", 0));
    } // retranslateUi

};

namespace Ui {
    class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GRADIENTEDITOR_H
