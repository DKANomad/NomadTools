#include "group-recordings.h"

#include <QtCore/qstring.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qboxlayout.h>

#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/util.hpp>

#include <util/platform.h>

QVBoxLayout *groupRecordingsDialogLayout;
QDialog *groupRecordingsDialog;
QPushButton *testButton;
ConfigFile basicConfig;

void MainDock::on_groupRecordingsButton_clicked()
{
	groupRecordingsDialog->show();
}

void stop(obs_frontend_event evt, void *private_data) {
	if (evt == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
		
	}
}

void ensure_directory_exists(std::string &path)
{
	replace(path.begin(), path.end(), '\\', '/');

	size_t last = path.rfind('/');
	if (last == std::string::npos)
		return;

	std::string directory = path.substr(0, last);
	os_mkdirs(directory.c_str());
}

// Use the same logic as OBSBasic to ascertain the current output path
const char *GroupRecordings::GetCurrentOutputPath(config_t* config)
{
	const char *path = nullptr;
	const char *mode = config_get_string(config, "Output", "Mode");

	if (strcmp(mode, "Advanced") == 0) {
		const char *advanced_mode =
			config_get_string(config, "AdvOut", "RecType");

		if (strcmp(advanced_mode, "FFmpeg") == 0) {
			path = config_get_string(config, "AdvOut",
						 "FFFilePath");
		} else {
			path = config_get_string(config, "AdvOut",
						 "RecFilePath");
		}
	} else {
		path = config_get_string(config, "SimpleOutput", "FilePath");
	}

	return path;
}

// Use the same logic as OBSBasic to ascertain the current output path and change it
void GroupRecordings::SetCurrentOutputPath(config_t* config, char* newPath) {
	const char *mode = config_get_string(config, "Output", "Mode");

	if (strcmp(mode, "Advanced") == 0) {
		const char *advanced_mode =
			config_get_string(config, "AdvOut", "RecType");

		if (strcmp(advanced_mode, "FFmpeg") == 0) {
			config_set_string(config, "AdvOut",
						 "FFFilePath", newPath);
		} else {
			config_set_string(config, "AdvOut",
						 "RecFilePath", newPath);
		}
	} else {
		config_set_string(config, "SimpleOutput", "FilePath", newPath);
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

	config_t *profileConfig = obs_frontend_get_profile_config();
	const char *outputPath = GetCurrentOutputPath(profileConfig);
	char fullVal[256] = "";
	strcpy(fullVal, outputPath);
	char *newVal = strcat(fullVal, "\\group-recordings");
	SetCurrentOutputPath(profileConfig, newVal);

	// We concat \\ to the end as it's needed for creating the directory
	strcat(newVal, "\\");
	ensure_directory_exists(std::string(newVal));
}
