#include "group-recordings.h"

#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qcombobox.h>

#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/util.hpp>

#include <util/platform.h>

QStringList GroupRecordings::GetDirectoryHistory() {
	QString history = QString(config_get_string(
		profileConfig, CONFIG_SECTION, "DirectoryHistory"));

	if (history.size() > 0) {
		return history.split(QChar(','));
	}

	return QStringList(GetCurrentDirectory());
}

void GroupRecordings::SetDirectoryHistory(QStringList historyList) {
	QString history = historyList.join(QChar(','));

	config_set_string(profileConfig, CONFIG_SECTION, "DirectoryHistory",
			  history.toStdString().c_str());

	config_save(profileConfig);
}

bool GroupRecordings::PluginCurrentlyEnabled() {
	return config_get_bool(profileConfig, CONFIG_SECTION, "Enabled");
}

void GroupRecordings::SetPluginCurrentlyEnabled(bool value)
{
	config_set_bool(profileConfig, CONFIG_SECTION, "Enabled", value);
	config_save(profileConfig);

	SetCurrentOutputPath(profileConfig, value);
}

void GroupRecordings::on_groupRecordingsButton_clicked() {
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
void GroupRecordings::SetCurrentOutputPath(config_t* config, bool enabled) {

	QString currentOutputFolder =
		QString(config_get_string(profileConfig, CONFIG_SECTION,
					  "CurrentDirectory"))
			.prepend("\\");

	QString currentOutputPath =
		QString(GetCurrentOutputPath(profileConfig));

	if (enabled) {
		currentOutputPath.append(currentOutputFolder);
		ChangeToggleText(true);
	} else {

		currentOutputPath.remove(currentOutputFolder);
		ChangeToggleText(false);
	}

	const char *mode = config_get_string(config, "Output", "Mode");

	if (strcmp(mode, "Advanced") == 0) {
		const char *advanced_mode =
			config_get_string(config, "AdvOut", "RecType");

		if (strcmp(advanced_mode, "FFmpeg") == 0) {
			config_set_string(
				config, "AdvOut", "FFFilePath",
				currentOutputPath.toStdString().c_str());
		} else {
			config_set_string(
				config, "AdvOut", "RecFilePath",
				currentOutputPath.toStdString().c_str());
		}
	} else {
		config_set_string(config, "SimpleOutput", "FilePath",
				  currentOutputPath.toStdString().c_str());
	}

	config_save(profileConfig);
}

void GroupRecordings::UpdateDirectoryHistory(QString newEntry) {
	QStringList historyList = GetDirectoryHistory();
	if (!historyList.contains(newEntry, Qt::CaseInsensitive)) {
		historyList.prepend(newEntry);
		if (historyList.count() > 5) {
			historyList.removeLast();
		}
	} else {
		int existingElIdx = historyList.indexOf(newEntry);
		historyList.move(existingElIdx, 0);
	}

	SetDirectoryHistory(historyList);
}

void GroupRecordings::ReorderHistoryDropdown() {
	groupRecordingsHistory->clear();
	groupRecordingsHistory->addItems(GetDirectoryHistory());
}

void GroupRecordings::VerifyDirectoryExists(QString folderName) {
	QString currentOuputPath = GetCurrentOutputPath(profileConfig);
	QDir outputDir = QDir(QString(currentOuputPath));
	outputDir.mkdir(folderName);

	outputDir.cd(folderName);
}

void GroupRecordings::On_SaveGroupRecording_Clicked() {
	QString newOutputFolderName = folderToAppend->text();

	// This is a hack for changing the dropdown when the plugin is enabled...I hate it
	// It also handles verifying the folder exists while changing the folder, because otherwise it nests in the old directory.
	if (PluginCurrentlyEnabled()) {
		SetCurrentOutputPath(profileConfig, false);
		SetCurrentDirectory(newOutputFolderName);
		VerifyDirectoryExists(newOutputFolderName);
		SetCurrentOutputPath(profileConfig, true);
	} else {
		VerifyDirectoryExists(newOutputFolderName);
		SetCurrentDirectory(newOutputFolderName);
	}

	

	groupRecordingsDialog->hide();
	UpdateDirectoryHistory(newOutputFolderName);
	ReorderHistoryDropdown();
}

void GroupRecordings::On_GroupRecordingToggle_Clicked() {
	if (PluginCurrentlyEnabled()) {
		SetPluginCurrentlyEnabled(false);
	} else {
		SetPluginCurrentlyEnabled(true);
	}
}

void GroupRecordings::SetCurrentDirectory(QString folderName) {
	config_set_string(profileConfig, CONFIG_SECTION, "CurrentDirectory",
			  folderName.toStdString().c_str());

	config_save(profileConfig);
}

QString GroupRecordings::GetCurrentDirectory()
{
	return QString(config_get_string(profileConfig, CONFIG_SECTION, "CurrentDirectory"));
}

void GroupRecordings::on_groupRecordingsHistory_Changed(QString text) {
	UpdateDirectoryHistory(text);

	// This is a hack for changing the dropdown when the plugin is enabled...I hate it
	// It also handles verifying the folder exists while changing the folder, because otherwise it nests in the old directory.
	if (PluginCurrentlyEnabled()) {
		SetCurrentOutputPath(profileConfig, false);
		SetCurrentDirectory(text);
		VerifyDirectoryExists(text);
		SetCurrentOutputPath(profileConfig, true);
	} else {
		VerifyDirectoryExists(text);
		SetCurrentDirectory(text);
	}

	ReorderHistoryDropdown();
}

void GroupRecordings::InitialiseDockElements() {
	groupRecordingsBoxLayout = new QHBoxLayout();
	groupRecordingsBoxLayout->addSpacing(0);
	groupRecordingsBoxLayout->setContentsMargins(QMargins(0, 0, 0, 0));

	QString currentDirectory = QString(config_get_string(
		profileConfig, CONFIG_SECTION, "CurrentDirectory"));

	QIcon cogIcon;
	cogIcon.addFile(
		QString::fromUtf8(":/settings/images/settings/general.svg"),
		QSize(), QIcon::Normal, QIcon::Off);

	QMenu *groupRecordingsMenu = new QMenu();
	QPushButton *groupRecordingsSettings = new QPushButton();

	QSizePolicy cogSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	cogSizePolicy.setHorizontalStretch(0);
	cogSizePolicy.setVerticalStretch(0);
	cogSizePolicy.setHeightForWidth(
		groupRecordingsSettings->sizePolicy().hasHeightForWidth());

	groupRecordingsSettings->setSizePolicy(cogSizePolicy);
	groupRecordingsSettings->setMaximumSize(QSize(22, 22));
	groupRecordingsSettings->setText(QString::fromUtf8(""));
	groupRecordingsSettings->setIcon(cogIcon);
	groupRecordingsSettings->setFlat(true);
	groupRecordingsSettings->setProperty(
		"themeID", QVariant(QString::fromUtf8("configIconSmall")));

	QStringList folderHistory = GetDirectoryHistory();
	
	groupRecordingsHistory = new QComboBox();
	groupRecordingsHistory->addItems(folderHistory);

	groupRecordingsBoxLayout->addWidget(groupRecordingsHistory);
	groupRecordingsBoxLayout->addWidget(groupRecordingsSettings);

	groupRecordingsButtonToggle = new QPushButton();
	groupRecordingsButtonToggle->setObjectName(
		"groupRecordingsButtonToggle");

	ChangeToggleText(PluginCurrentlyEnabled());

	// Connects the group recordings tool button to the MainDock event handler.
	QWidget::connect(groupRecordingsSettings, &QPushButton::clicked,
			 this, &GroupRecordings::on_groupRecordingsButton_clicked);

	QWidget::connect(groupRecordingsButtonToggle, &QPushButton::clicked,
			 this,
			 &GroupRecordings::On_GroupRecordingToggle_Clicked);

	QWidget::connect(groupRecordingsHistory, &QComboBox::textActivated,
			 this,
			 &GroupRecordings::on_groupRecordingsHistory_Changed);
}

void GroupRecordings::InitialiseDialog(MainDock *mainDock) {
	groupRecordingsDialog = new QDialog(mainDock);
	groupRecordingsDialog->setWindowTitle(
		QString::fromUtf8("Group Recordings"));
	groupRecordingsDialog->setFixedSize(500, 200);

	QFormLayout *formLayout = new QFormLayout();
	QLabel *folderToAppendLabel =
		new QLabel(QString::fromUtf8("Folder name"));
	folderToAppend = new QLineEdit();

	formLayout->addRow(folderToAppendLabel, folderToAppend);

	QVBoxLayout* groupRecordingsDialogLayout = new QVBoxLayout(groupRecordingsDialog);
	groupRecordingsDialogLayout->addLayout(formLayout);

	QPushButton *saveGroupRecordingButton =
		new QPushButton(QString::fromUtf8("Save Changes"));
	groupRecordingsDialogLayout->addWidget(saveGroupRecordingButton);

	// Connects save button to callback
	QWidget::connect(saveGroupRecordingButton, &QPushButton::clicked, this,
			 &GroupRecordings::On_SaveGroupRecording_Clicked);
}

void GroupRecordings::InitializePlugin(MainDock *mainDock) {
	profileConfig = obs_frontend_get_profile_config();

	InitialiseDockElements();
	InitialiseDialog(mainDock);	
}
