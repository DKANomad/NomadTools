#include "nomad-tools.h"
#include "group-recordings.h"

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("nomad-tools", "en-US")

bool obs_module_load(void)
{
	// Main housing dock for all custom tools.
	MainDock *mainDock = new MainDock();
	mainDock->setWindowTitle(QString::fromUtf8("Nomad Tools"));
	mainDock->setObjectName(QString::fromUtf8("nomadTools"));
	mainDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	mainDock->setFloating(false);
	mainDock->setEnabled(true);
	mainDock->setVisible(true);

	QWidget *mainDockContents = new QWidget(mainDock);

	GroupRecordings *groupRecordingsPlugin = new GroupRecordings();
	groupRecordingsPlugin->InitializePlugin(mainDock);

	QPushButton *groupRecordingsButton = new QPushButton(
		QString::fromUtf8("Group Recordings"));
	groupRecordingsButton->setObjectName("groupRecordingsButton");

	QPushButton* groupRecordingsButtonToggle = new QPushButton(
		QString::fromUtf8("Enable Group Recording"));
	groupRecordingsButtonToggle->setObjectName("groupRecordingsButtonToggle");

	QVBoxLayout *mainBoxLayout = new QVBoxLayout(mainDockContents);
	mainBoxLayout->setSpacing(2);
	mainBoxLayout->setAlignment(Qt::AlignTop);
	mainBoxLayout->addWidget(groupRecordingsButton);
	mainBoxLayout->addWidget(groupRecordingsButtonToggle);

	QMainWindow *mainWindow = (QMainWindow *)obs_frontend_get_main_window();
	mainWindow->addDockWidget(Qt::BottomDockWidgetArea, mainDock);

	// Connects the group recordings tool button to the MainDock event handler.
	QWidget::connect(groupRecordingsButton, &QPushButton::clicked, mainDock,
			 &MainDock::on_groupRecordingsButton_clicked);

	QWidget::connect(groupRecordingsButtonToggle, &QPushButton::clicked,
			 groupRecordingsPlugin,
			 &GroupRecordings::On_GroupRecordingToggle_Clicked);

	mainDock->setWidget(mainDockContents);
	obs_frontend_add_dock(mainDock);

	return true;
}
