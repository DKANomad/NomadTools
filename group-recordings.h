#pragma once

#include "nomad-tools.h"
#include <obs-frontend-api.h>

class GroupRecordings : public QObject {
public:
	void InitializePlugin(MainDock *mainDock);
	const char *GetCurrentOutputPath(config_t *config);
	void SetCurrentOutputPath(config_t *config, const char* newPath);

	void On_SaveGroupRecording_Clicked();
};
