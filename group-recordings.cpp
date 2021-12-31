#include "group-recordings.h"

#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qformlayout.h>

#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/util.hpp>

#include <util/platform.h>

QVBoxLayout *groupRecordingsDialogLayout;
QDialog *groupRecordingsDialog;
QPushButton *saveGroupRecordingButton;
ConfigFile basicConfig;
QLineEdit *folderToAppend;
config_t *profileConfig;

const char *CONFIG_SECTION = "NomadTools.GroupRecording";

void MainDock::on_groupRecordingsButton_clicked()
{
	groupRecordingsDialog->show();
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
void GroupRecordings::SetCurrentOutputPath(config_t* config, const char* newPath) {
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

void GroupRecordings::On_SaveGroupRecording_Clicked() {

	QString newOutputFolderName = folderToAppend->text();

	QDir outputDir = QDir(QString(GetCurrentOutputPath(profileConfig)));
	outputDir.mkdir(newOutputFolderName);
	outputDir.cd(newOutputFolderName);

	config_set_string(profileConfig, CONFIG_SECTION, "CurrentDirectory",
			  newOutputFolderName.toStdString().c_str());
}

void GroupRecordings::On_GroupRecordingToggle_Clicked() {
	QString currentOutputFolder = QString(config_get_string(
		profileConfig, CONFIG_SECTION, "CurrentDirectory"));

	QString currentOutputPath =
		QString(GetCurrentOutputPath(profileConfig));

	if (currentOutputPath.contains(currentOutputFolder)) {
		currentOutputFolder.remove(currentOutputFolder.prepend("\\"));
	} else {
		currentOutputFolder.append(currentOutputFolder.prepend("\\"));
	}

	SetCurrentOutputPath(profileConfig,
			     currentOutputPath.toStdString().c_str());
}

void GroupRecordings::InitializePlugin(MainDock *mainDock) {

	profileConfig = obs_frontend_get_profile_config();

	if (config_get_bool(profileConfig, CONFIG_SECTION, "Enabled") == NULL) {
		config_set_bool(profileConfig, CONFIG_SECTION, "Enabled",
				false);
	}

	groupRecordingsDialog = new QDialog(mainDock);
	groupRecordingsDialog->setWindowTitle(
		QString::fromUtf8("Group Recordings"));
	groupRecordingsDialog->setFixedSize(500, 200);

	QFormLayout *formLayout = new QFormLayout();
	QLabel *folderToAppendLabel =
		new QLabel(QString::fromUtf8("Folder name"));
	folderToAppend = new QLineEdit();

	formLayout->addRow(folderToAppendLabel, folderToAppend);

	groupRecordingsDialogLayout = new QVBoxLayout(groupRecordingsDialog);
	groupRecordingsDialogLayout->addLayout(formLayout);

	saveGroupRecordingButton = new QPushButton(QString::fromUtf8("Save Changes"));
	groupRecordingsDialogLayout->addWidget(saveGroupRecordingButton);

	// Connects save button to callback
	QWidget::connect(saveGroupRecordingButton, &QPushButton::clicked, this,
			 &GroupRecordings::On_SaveGroupRecording_Clicked);
}
