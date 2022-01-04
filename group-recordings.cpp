#include "group-recordings.h"

#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
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

const char *CONFIG_SECTION = "NomadTools.GroupRecordings";

bool PluginCurrentlyEnabled()
{
	return config_get_bool(profileConfig, CONFIG_SECTION, "Enabled");
}

void SetPluginCurrentlyEnabled(bool value)
{
	config_set_bool(profileConfig, CONFIG_SECTION, "Enabled", value);
}

void MainDock::on_groupRecordingsButton_clicked() {
	QString currentDirectory = QString(
		config_get_string(profileConfig, CONFIG_SECTION,
				  "CurrentDirectory"));

	groupRecordingsDialog->show();
	folderToAppend->setText(currentDirectory);
}

void GroupRecordings::ChangeToggleText(bool enabled) {
	if (enabled) {
		groupRecordingsButtonToggle->setText(
			QString::fromUtf8("Disable Group Recording"));
	} else {
		groupRecordingsButtonToggle->setText(
			QString::fromUtf8("Enable Group Recording"));
	}
}

// Use the same logic as OBSBasic to ascertain the current output path
const char *GroupRecordings::GetCurrentOutputPath(config_t* config) {
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

	config_save(profileConfig);
}

void GroupRecordings::On_SaveGroupRecording_Clicked() {
	QString newOutputFolderName = folderToAppend->text();

	QDir outputDir = QDir(QString(GetCurrentOutputPath(profileConfig)));
	outputDir.mkdir(newOutputFolderName);
	outputDir.cd(newOutputFolderName);

	config_set_string(profileConfig, CONFIG_SECTION, "CurrentDirectory",
			  newOutputFolderName.toStdString().c_str());

	config_save(profileConfig);
}

void GroupRecordings::On_GroupRecordingToggle_Clicked() {
	QString currentOutputFolder = QString(config_get_string(
		profileConfig, CONFIG_SECTION, "CurrentDirectory")).prepend("\\");

	QString currentOutputPath =
		QString(GetCurrentOutputPath(profileConfig));

	if (PluginCurrentlyEnabled()) {
		currentOutputPath.remove(currentOutputFolder);
		SetPluginCurrentlyEnabled(false);
		ChangeToggleText(false);
	} else {
		currentOutputPath.append(currentOutputFolder);
		SetPluginCurrentlyEnabled(true);
		ChangeToggleText(true);
	}

	config_save(profileConfig);

	SetCurrentOutputPath(profileConfig,
			     currentOutputPath.toStdString().c_str());
}

void GroupRecordings::InitializePlugin(MainDock *mainDock) {

	profileConfig = obs_frontend_get_profile_config();

	groupRecordingsButton =
		new QPushButton(QString::fromUtf8("Group Recordings"));
	groupRecordingsButton->setObjectName("groupRecordingsButton");

	groupRecordingsButtonToggle = new QPushButton();
	ChangeToggleText(PluginCurrentlyEnabled());

	groupRecordingsButtonToggle->setObjectName(
		"groupRecordingsButtonToggle");

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

	// Connects the group recordings tool button to the MainDock event handler.
	QWidget::connect(groupRecordingsButton, &QPushButton::clicked, mainDock,
			 &MainDock::on_groupRecordingsButton_clicked);

	QWidget::connect(groupRecordingsButtonToggle, &QPushButton::clicked,
			 this,
			 &GroupRecordings::On_GroupRecordingToggle_Clicked);
}
