#include "group-recordings.h"

#include <QVBoxLayout>
#include <QWidget>

#include <obs-frontend-api.h>
#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("nomad-tools", "en-US")

namespace {
constexpr char DOCK_ID[] = "nomadTools";
GroupRecordings *groupRecordingsPlugin = nullptr;

void OnFrontendEvent(enum obs_frontend_event event, void *privateData)
{
	auto *plugin = static_cast<GroupRecordings *>(privateData);
	plugin->HandleFrontendEvent(event);
}
} // namespace

bool obs_module_load(void)
{
	auto *mainDockContents = new QWidget();
	auto *plugin = new GroupRecordings();
	plugin->InitializePlugin(mainDockContents);

	auto *mainBoxLayout = new QVBoxLayout(mainDockContents);
	mainBoxLayout->setSpacing(1);
	mainBoxLayout->setAlignment(Qt::AlignTop);
	mainBoxLayout->setContentsMargins(QMargins(2, 4, 2, 4));
	auto *boxLayoutContainer = new QWidget(mainDockContents);
	boxLayoutContainer->setLayout(plugin->groupRecordingsBoxLayout);

	mainBoxLayout->addWidget(boxLayoutContainer);
	mainBoxLayout->addWidget(plugin->groupRecordingsButtonToggle);

	if (!obs_frontend_add_dock_by_id(DOCK_ID, obs_module_text("NomadTools.DockTitle"), mainDockContents)) {
		delete plugin;
		delete mainDockContents;
		return false;
	}

	groupRecordingsPlugin = plugin;
	obs_frontend_add_event_callback(OnFrontendEvent, groupRecordingsPlugin);

	return true;
}

void obs_module_unload(void)
{
	if (!groupRecordingsPlugin) {
		return;
	}

	obs_frontend_remove_event_callback(OnFrontendEvent, groupRecordingsPlugin);
	groupRecordingsPlugin->SetPluginCurrentlyEnabled(false);
	delete groupRecordingsPlugin;
	groupRecordingsPlugin = nullptr;
	obs_frontend_remove_dock(DOCK_ID);
}
