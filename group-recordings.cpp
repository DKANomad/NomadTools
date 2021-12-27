#include "group-recordings.h"

#include <QtCore/qstring.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qboxlayout.h>

#include <obs-frontend-api.h>

QVBoxLayout *groupRecordingsDialogLayout;
QDialog *groupRecordingsDialog;
QPushButton *testButton;

void MainDock::on_groupRecordingsButton_clicked()
{
	groupRecordingsDialog->show();
}

void stop(obs_frontend_event evt, void *private_data) {
	if (evt == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
		std::string stopper = "hello";
	}
}

void GroupRecordings::InitializePlugin(MainDock *mainDock) {

	// Binds the stop callback to obs frontend event
	obs_frontend_event_cb cb = obs_frontend_event_cb(stop);
	obs_frontend_add_event_callback(cb, "");

	groupRecordingsDialog = new QDialog(mainDock);
	groupRecordingsDialog->setWindowTitle(
		QString::fromUtf8("Group Recordings"));
	groupRecordingsDialog->setFixedSize(1000, 1000);

	groupRecordingsDialogLayout = new QVBoxLayout(groupRecordingsDialog);

	testButton = new QPushButton(QString::fromUtf8("Testing"));
	groupRecordingsDialogLayout->addWidget(testButton);
}
