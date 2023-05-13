#include "nomad-tools.h"
#include "group-recordings.h"

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qboxlayout.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("nomad-tools", "en-US")

GroupRecordings *groupRecordingsPlugin;

void OnClose(enum obs_frontend_event evt, void* data) {
	if (evt == OBS_FRONTEND_EVENT_EXIT) {
		
		// Before OBS exits, always set the plugin to disabled and reset the path to original value
		groupRecordingsPlugin->SetPluginCurrentlyEnabled(false);
	}
}

bool obs_module_load(void)
{
	// Main housing dock for all custom tools.
	MainDock *mainDock = new MainDock();
	mainDock->setWindowTitle(QString::fromUtf8("Nomad Tools"));
	mainDock->setObjectName(QString::fromUtf8("nomadTools"));
	mainDock->setFloating(false);
	mainDock->setEnabled(true);
	mainDock->setVisible(true);

	QWidget *mainDockContents = new QWidget(mainDock);

	groupRecordingsPlugin = new GroupRecordings();
	groupRecordingsPlugin->InitializePlugin(mainDock);

	QVBoxLayout *mainBoxLayout = new QVBoxLayout(mainDockContents);
	mainBoxLayout->setSpacing(1);
	mainBoxLayout->setAlignment(Qt::AlignTop);
	mainBoxLayout->setContentsMargins(QMargins(2, 4, 2, 4));
	QWidget *boxLayoutContainer = new QWidget();
	boxLayoutContainer->setLayout(
		groupRecordingsPlugin->groupRecordingsBoxLayout);

	mainBoxLayout->addWidget(boxLayoutContainer);
	mainBoxLayout->addWidget(groupRecordingsPlugin->groupRecordingsButtonToggle);

	QMainWindow *mainWindow = (QMainWindow *)obs_frontend_get_main_window();
	mainWindow->addDockWidget(Qt::BottomDockWidgetArea, mainDock);

	mainDock->setWidget(mainDockContents);
	obs_frontend_add_dock(mainDock);
	obs_frontend_add_event_callback(OnClose, NULL);

	return true;
}
