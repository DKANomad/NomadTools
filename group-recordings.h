#pragma once

#include <QComboBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QStringList>

#include <obs-frontend-api.h>

class GroupRecordings : public QObject {
public:
	void InitializePlugin(QWidget *parent);
	void HandleFrontendEvent(enum obs_frontend_event event);
	bool PluginCurrentlyEnabled();
	void SetPluginCurrentlyEnabled(bool value);

	QPushButton *groupRecordingsButtonToggle = nullptr;
	QHBoxLayout *groupRecordingsBoxLayout = nullptr;

private:
	void ChangeToggleText(bool enabled);
	void InitializeProfile();
	void InitialiseDockElements();
	void InitialiseDialog(QWidget *parent);
	void OnSaveGroupRecordingClicked();
	void OnGroupRecordingToggleClicked();
	void OnGroupRecordingsHistoryChanged(const QString &text);
	void OpenGroupRecordingsDialog(bool enableAfterSave = false);

	QString GetCurrentOutputPath(config_t *config) const;
	bool SetCurrentOutputPath(config_t *config, bool enabled);
	bool VerifyDirectoryExists(const QString &folderName);

	QStringList GetDirectoryHistory();
	void SetDirectoryHistory(const QStringList &historyList);
	void UpdateDirectoryHistory(const QString &newEntry);
	void ReorderHistoryDropdown();
	void SetCurrentDirectory(const QString &folderName);
	QString GetCurrentDirectory();

	QDialog *groupRecordingsDialog = nullptr;
	QLineEdit *folderToAppend = nullptr;
	QComboBox *groupRecordingsHistory = nullptr;
	bool enableAfterSave = false;
};
