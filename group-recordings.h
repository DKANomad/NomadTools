#pragma once

#include "nomad-tools.h"
#include <obs-frontend-api.h>

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlineedit.h>

class GroupRecordings : public QObject {
public:
	const char *CONFIG_SECTION = "NomadTools.GroupRecordings";
	config_t *profileConfig;

	void InitializePlugin(MainDock *mainDock);
	const char *GetCurrentOutputPath(config_t *config);
	void SetCurrentOutputPath(config_t *config, bool enabled);

	void On_SaveGroupRecording_Clicked();
	void On_GroupRecordingToggle_Clicked();

	void ChangeToggleText(bool enabled);

	bool PluginCurrentlyEnabled();
	void SetPluginCurrentlyEnabled(bool value);

	void InitialiseDockElements();
	void InitialiseDialog(MainDock *mainDock);

	void on_groupRecordingsButton_clicked();
	void on_groupRecordingsHistory_Changed(QString text);

	void UpdateDirectoryHistory(QString newEntry);
	void SetCurrentDirectory(QString folderName);
	QString GetCurrentDirectory();

	void ReorderHistoryDropdown();
	void VerifyDirectoryExists(QString folderName);

	QStringList GetDirectoryHistory();
	void SetDirectoryHistory(QStringList historyList);

	QPushButton *groupRecordingsButtonToggle;
	QDialog *groupRecordingsDialog;
	QLineEdit *folderToAppend;
	QComboBox *groupRecordingsHistory;

	QHBoxLayout *groupRecordingsBoxLayout;
};
