#pragma once

#include "nomad-tools.h"
#include <obs-frontend-api.h>

#include <QtWidgets/qpushbutton.h>

class GroupRecordings : public QObject {
public:
	void InitializePlugin(MainDock *mainDock);
	const char *GetCurrentOutputPath(config_t *config);
	void SetCurrentOutputPath(config_t *config, const char* newPath);

	void On_SaveGroupRecording_Clicked();
	void On_GroupRecordingToggle_Clicked();

	void ChangeToggleText(bool enabled);

	bool PluginCurrentlyEnabled();
	void SetPluginCurrentlyEnabled(bool value);

	QPushButton *groupRecordingsButtonToggle;
	QPushButton *groupRecordingsButton;
};
