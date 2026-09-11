#include "group-recordings.h"

#include <QDir>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <cstring>

#include <obs-module.h>
#include <util/config-file.h>

namespace {
constexpr char CONFIG_SECTION[] = "NomadTools.GroupRecordings";
constexpr char CURRENT_DIRECTORY[] = "CurrentDirectory";
constexpr char DIRECTORY_HISTORY[] = "DirectoryHistory";
constexpr char ENABLED[] = "Enabled";
constexpr char ORIGINAL_OUTPUT_PATH[] = "OriginalOutputPath";
constexpr char ORIGINAL_OUTPUT_SECTION[] = "OriginalOutputSection";
constexpr char ORIGINAL_OUTPUT_KEY[] = "OriginalOutputKey";

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
constexpr Qt::CaseSensitivity FILESYSTEM_CASE_SENSITIVITY = Qt::CaseInsensitive;
#else
constexpr Qt::CaseSensitivity FILESYSTEM_CASE_SENSITIVITY = Qt::CaseSensitive;
#endif

struct OutputPathSetting {
	const char *section;
	const char *key;
};

QString ConfigString(config_t *config, const char *section, const char *key)
{
	const char *value = config_get_string(config, section, key);
	return QString::fromUtf8(value ? value : "");
}

void SetConfigString(config_t *config, const char *section, const char *key, const QString &value)
{
	const QByteArray utf8 = value.toUtf8();
	config_set_string(config, section, key, utf8.constData());
}

OutputPathSetting GetOutputPathSetting(config_t *config)
{
	const char *mode = config_get_string(config, "Output", "Mode");
	if (mode && strcmp(mode, "Advanced") == 0) {
		const char *recordingType = config_get_string(config, "AdvOut", "RecType");
		if (recordingType && strcmp(recordingType, "FFmpeg") == 0) {
			return {"AdvOut", "FFFilePath"};
		}

		return {"AdvOut", "RecFilePath"};
	}

	return {"SimpleOutput", "FilePath"};
}

bool IsOutputPathSetting(const QString &section, const QString &key)
{
	return (section == "SimpleOutput" && key == "FilePath") ||
	       (section == "AdvOut" && (key == "RecFilePath" || key == "FFFilePath"));
}

bool NormalizeFolderName(const QString &folderName, QString &normalizedFolderName)
{
	normalizedFolderName = QDir::cleanPath(QDir::fromNativeSeparators(folderName.trimmed()));
	return !normalizedFolderName.isEmpty() && normalizedFolderName != "." && normalizedFolderName != ".." &&
	       !normalizedFolderName.startsWith("../") && !QDir::isAbsolutePath(normalizedFolderName) &&
	       !normalizedFolderName.contains(QChar(','));
}

QString NormalizedPath(const QString &path)
{
	return QDir::cleanPath(QDir::fromNativeSeparators(path));
}

void SaveConfig(config_t *config)
{
	config_save_safe(config, "tmp", nullptr);
}
} // namespace

QStringList GroupRecordings::GetDirectoryHistory()
{
	config_t *config = obs_frontend_get_profile_config();
	if (!config) {
		return {};
	}

	const QString history = ConfigString(config, CONFIG_SECTION, DIRECTORY_HISTORY);
	if (!history.isEmpty()) {
		return history.split(QChar(','));
	}

	const QString currentDirectory = GetCurrentDirectory();
	return currentDirectory.isEmpty() ? QStringList{} : QStringList{currentDirectory};
}

void GroupRecordings::SetDirectoryHistory(const QStringList &historyList)
{
	config_t *config = obs_frontend_get_profile_config();
	if (!config) {
		return;
	}

	SetConfigString(config, CONFIG_SECTION, DIRECTORY_HISTORY, historyList.join(QChar(',')));
	SaveConfig(config);
}

bool GroupRecordings::PluginCurrentlyEnabled()
{
	config_t *config = obs_frontend_get_profile_config();
	return config && config_get_bool(config, CONFIG_SECTION, ENABLED);
}

void GroupRecordings::SetPluginCurrentlyEnabled(bool value)
{
	config_t *config = obs_frontend_get_profile_config();
	if (!config) {
		return;
	}

	const bool currentlyEnabled = config_get_bool(config, CONFIG_SECTION, ENABLED);
	if (currentlyEnabled == value) {
		ChangeToggleText(value);
		return;
	}

	if (!SetCurrentOutputPath(config, value)) {
		return;
	}

	config_set_bool(config, CONFIG_SECTION, ENABLED, value);
	SaveConfig(config);
	ChangeToggleText(value);
}

void GroupRecordings::OpenGroupRecordingsDialog(bool enableAfterSave)
{
	this->enableAfterSave = enableAfterSave;
	groupRecordingsDialog->show();
	folderToAppend->setText(GetCurrentDirectory());
}

void GroupRecordings::ChangeToggleText(bool enabled)
{
	if (!groupRecordingsButtonToggle) {
		return;
	}

	const char *key = enabled ? "GroupRecordings.Disable" : "GroupRecordings.Enable";
	groupRecordingsButtonToggle->setText(QString::fromUtf8(obs_module_text(key)));
}

QString GroupRecordings::GetCurrentOutputPath(config_t *config) const
{
	const OutputPathSetting setting = GetOutputPathSetting(config);
	return ConfigString(config, setting.section, setting.key);
}

bool GroupRecordings::SetCurrentOutputPath(config_t *config, bool enabled)
{
	if (enabled) {
		const QString outputPath = GetCurrentOutputPath(config);
		QString folderName;

		if (outputPath.isEmpty() || !NormalizeFolderName(GetCurrentDirectory(), folderName) ||
		    !VerifyDirectoryExists(folderName)) {
			return false;
		}

		const OutputPathSetting setting = GetOutputPathSetting(config);
		SetConfigString(config, CONFIG_SECTION, ORIGINAL_OUTPUT_PATH, outputPath);
		config_set_string(config, CONFIG_SECTION, ORIGINAL_OUTPUT_SECTION, setting.section);
		config_set_string(config, CONFIG_SECTION, ORIGINAL_OUTPUT_KEY, setting.key);
		SetConfigString(config, setting.section, setting.key,
				QDir::cleanPath(QDir(outputPath).filePath(folderName)));
		return true;
	}

	if (config_has_user_value(config, CONFIG_SECTION, ORIGINAL_OUTPUT_PATH)) {
		const QString section = ConfigString(config, CONFIG_SECTION, ORIGINAL_OUTPUT_SECTION);
		const QString key = ConfigString(config, CONFIG_SECTION, ORIGINAL_OUTPUT_KEY);
		const QString originalPath = ConfigString(config, CONFIG_SECTION, ORIGINAL_OUTPUT_PATH);
		QString folderName;
		if (IsOutputPathSetting(section, key) && NormalizeFolderName(GetCurrentDirectory(), folderName)) {
			const QByteArray sectionUtf8 = section.toUtf8();
			const QByteArray keyUtf8 = key.toUtf8();
			const QString currentPath = ConfigString(config, sectionUtf8.constData(), keyUtf8.constData());
			const QString groupedPath = QDir::cleanPath(QDir(originalPath).filePath(folderName));
			if (NormalizedPath(currentPath)
				    .compare(NormalizedPath(groupedPath), FILESYSTEM_CASE_SENSITIVITY) == 0) {
				SetConfigString(config, sectionUtf8.constData(), keyUtf8.constData(), originalPath);
			}
		}
	} else {
		// Recover paths written by versions that did not persist the original value.
		const OutputPathSetting setting = GetOutputPathSetting(config);
		QString outputPath = ConfigString(config, setting.section, setting.key);
		QString folderName;
		if (!NormalizeFolderName(GetCurrentDirectory(), folderName)) {
			folderName.clear();
		}
		const QString normalizedSuffix = QStringLiteral("/") + folderName;
		QString normalizedPath = QDir::fromNativeSeparators(outputPath);

		if (!folderName.isEmpty() && normalizedPath.endsWith(normalizedSuffix, FILESYSTEM_CASE_SENSITIVITY)) {
			QString originalPath = normalizedPath;
			originalPath.chop(normalizedSuffix.size());
#ifdef Q_OS_WIN
			if (originalPath.size() == 2 && originalPath.at(1) == QChar(':')) {
				originalPath.append(QChar('/'));
			}
#endif
			if (!originalPath.isEmpty() && QDir(originalPath).exists()) {
				outputPath = QDir::toNativeSeparators(originalPath);
			}
		} else {
			const QString legacySuffix = QStringLiteral("\\") + folderName;
			if (!folderName.isEmpty() && outputPath.endsWith(legacySuffix, FILESYSTEM_CASE_SENSITIVITY)) {
				const QString originalPath = outputPath.first(outputPath.size() - legacySuffix.size());
				if (!originalPath.isEmpty() && QDir(originalPath).exists()) {
					outputPath = originalPath;
				}
			}
		}

		SetConfigString(config, setting.section, setting.key, outputPath);
	}

	config_remove_value(config, CONFIG_SECTION, ORIGINAL_OUTPUT_PATH);
	config_remove_value(config, CONFIG_SECTION, ORIGINAL_OUTPUT_SECTION);
	config_remove_value(config, CONFIG_SECTION, ORIGINAL_OUTPUT_KEY);
	return true;
}

void GroupRecordings::UpdateDirectoryHistory(const QString &newEntry)
{
	QStringList historyList = GetDirectoryHistory();
	qsizetype existingIndex = -1;
	for (qsizetype index = 0; index < historyList.size(); ++index) {
		if (historyList.at(index).compare(newEntry, FILESYSTEM_CASE_SENSITIVITY) == 0) {
			existingIndex = index;
			break;
		}
	}

	if (existingIndex < 0) {
		historyList.prepend(newEntry);
		if (historyList.count() > 5) {
			historyList.removeLast();
		}
	} else {
		historyList.move(existingIndex, 0);
	}

	SetDirectoryHistory(historyList);
}

void GroupRecordings::ReorderHistoryDropdown()
{
	groupRecordingsHistory->clear();
	groupRecordingsHistory->addItems(GetDirectoryHistory());
}

bool GroupRecordings::VerifyDirectoryExists(const QString &folderName)
{
	config_t *config = obs_frontend_get_profile_config();
	if (!config) {
		return false;
	}

	const QString outputPath = GetCurrentOutputPath(config);
	return !outputPath.isEmpty() && QDir(outputPath).mkpath(folderName);
}

void GroupRecordings::OnSaveGroupRecordingClicked()
{
	QString newOutputFolderName;
	if (!NormalizeFolderName(folderToAppend->text(), newOutputFolderName)) {
		QMessageBox::warning(groupRecordingsDialog,
				     QString::fromUtf8(obs_module_text("GroupRecordings.ErrorTitle")),
				     QString::fromUtf8(obs_module_text("GroupRecordings.DirectoryError")));
		return;
	}

	const bool wasEnabled = PluginCurrentlyEnabled();
	if (wasEnabled) {
		SetPluginCurrentlyEnabled(false);
	}

	if (!VerifyDirectoryExists(newOutputFolderName)) {
		if (wasEnabled) {
			SetPluginCurrentlyEnabled(true);
		}
		QMessageBox::warning(groupRecordingsDialog,
				     QString::fromUtf8(obs_module_text("GroupRecordings.ErrorTitle")),
				     QString::fromUtf8(obs_module_text("GroupRecordings.DirectoryError")));
		return;
	}

	SetCurrentDirectory(newOutputFolderName);
	if (wasEnabled || enableAfterSave) {
		SetPluginCurrentlyEnabled(true);
	}
	if ((wasEnabled || enableAfterSave) && !PluginCurrentlyEnabled()) {
		QMessageBox::warning(groupRecordingsDialog,
				     QString::fromUtf8(obs_module_text("GroupRecordings.ErrorTitle")),
				     QString::fromUtf8(obs_module_text("GroupRecordings.DirectoryError")));
		return;
	}
	enableAfterSave = false;

	groupRecordingsDialog->hide();
	UpdateDirectoryHistory(newOutputFolderName);
	ReorderHistoryDropdown();
}

void GroupRecordings::OnGroupRecordingToggleClicked()
{
	const bool enable = !PluginCurrentlyEnabled();
	SetPluginCurrentlyEnabled(enable);
	if (enable && !PluginCurrentlyEnabled()) {
		OpenGroupRecordingsDialog(true);
	}
}

void GroupRecordings::SetCurrentDirectory(const QString &folderName)
{
	config_t *config = obs_frontend_get_profile_config();
	if (!config) {
		return;
	}

	SetConfigString(config, CONFIG_SECTION, CURRENT_DIRECTORY, folderName);
	SaveConfig(config);
}

QString GroupRecordings::GetCurrentDirectory()
{
	config_t *config = obs_frontend_get_profile_config();
	return config ? ConfigString(config, CONFIG_SECTION, CURRENT_DIRECTORY) : QString{};
}

void GroupRecordings::OnGroupRecordingsHistoryChanged(const QString &text)
{
	QString folderName;
	if (!NormalizeFolderName(text, folderName)) {
		return;
	}

	const bool wasEnabled = PluginCurrentlyEnabled();
	if (wasEnabled) {
		SetPluginCurrentlyEnabled(false);
	}

	if (!VerifyDirectoryExists(folderName)) {
		if (wasEnabled) {
			SetPluginCurrentlyEnabled(true);
		}
		return;
	}

	SetCurrentDirectory(folderName);
	if (wasEnabled) {
		SetPluginCurrentlyEnabled(true);
	}

	UpdateDirectoryHistory(folderName);
	ReorderHistoryDropdown();
}

void GroupRecordings::InitialiseDockElements()
{
	groupRecordingsBoxLayout = new QHBoxLayout();
	groupRecordingsBoxLayout->setContentsMargins(QMargins(0, 0, 0, 0));

	QIcon cogIcon;
	cogIcon.addFile(QStringLiteral(":/settings/images/settings/general.svg"), QSize(), QIcon::Normal, QIcon::Off);

	auto *groupRecordingsSettings = new QPushButton();
	QSizePolicy cogSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	cogSizePolicy.setHeightForWidth(groupRecordingsSettings->sizePolicy().hasHeightForWidth());
	groupRecordingsSettings->setSizePolicy(cogSizePolicy);
	groupRecordingsSettings->setMaximumSize(QSize(22, 22));
	groupRecordingsSettings->setIcon(cogIcon);
	groupRecordingsSettings->setFlat(true);

	groupRecordingsHistory = new QComboBox();
	groupRecordingsHistory->addItems(GetDirectoryHistory());

	groupRecordingsBoxLayout->addWidget(groupRecordingsHistory);
	groupRecordingsBoxLayout->addWidget(groupRecordingsSettings);

	groupRecordingsButtonToggle = new QPushButton();
	groupRecordingsButtonToggle->setObjectName(QStringLiteral("groupRecordingsButtonToggle"));
	ChangeToggleText(PluginCurrentlyEnabled());

	QObject::connect(groupRecordingsSettings, &QPushButton::clicked, this,
			 &GroupRecordings::OpenGroupRecordingsDialog);
	QObject::connect(groupRecordingsButtonToggle, &QPushButton::clicked, this,
			 &GroupRecordings::OnGroupRecordingToggleClicked);
	QObject::connect(groupRecordingsHistory, &QComboBox::textActivated, this,
			 &GroupRecordings::OnGroupRecordingsHistoryChanged);
}

void GroupRecordings::InitialiseDialog(QWidget *parent)
{
	groupRecordingsDialog = new QDialog(parent);
	groupRecordingsDialog->setWindowTitle(QString::fromUtf8(obs_module_text("GroupRecordings.Title")));
	groupRecordingsDialog->setFixedSize(500, 200);

	auto *formLayout = new QFormLayout();
	auto *folderToAppendLabel = new QLabel(QString::fromUtf8(obs_module_text("GroupRecordings.FolderName")));
	folderToAppend = new QLineEdit();
	formLayout->addRow(folderToAppendLabel, folderToAppend);

	auto *dialogLayout = new QVBoxLayout(groupRecordingsDialog);
	dialogLayout->addLayout(formLayout);

	auto *saveButton = new QPushButton(QString::fromUtf8(obs_module_text("GroupRecordings.SaveChanges")));
	dialogLayout->addWidget(saveButton);
	QObject::connect(saveButton, &QPushButton::clicked, this, &GroupRecordings::OnSaveGroupRecordingClicked);
}

void GroupRecordings::InitializeProfile()
{
	config_t *config = obs_frontend_get_profile_config();
	if (!config) {
		return;
	}

	config_set_default_string(config, CONFIG_SECTION, CURRENT_DIRECTORY, "");
	config_set_default_string(config, CONFIG_SECTION, DIRECTORY_HISTORY, "");
	config_set_default_bool(config, CONFIG_SECTION, ENABLED, false);

	if (config_get_bool(config, CONFIG_SECTION, ENABLED)) {
		SetCurrentOutputPath(config, false);
		config_set_bool(config, CONFIG_SECTION, ENABLED, false);
		SaveConfig(config);
	}

	if (groupRecordingsHistory) {
		ReorderHistoryDropdown();
	}
	ChangeToggleText(false);
}

void GroupRecordings::HandleFrontendEvent(enum obs_frontend_event event)
{
	if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGING || event == OBS_FRONTEND_EVENT_EXIT) {
		SetPluginCurrentlyEnabled(false);
	} else if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED) {
		InitializeProfile();
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTING && PluginCurrentlyEnabled()) {
		SetPluginCurrentlyEnabled(false);
		SetPluginCurrentlyEnabled(true);
	}
}

void GroupRecordings::InitializePlugin(QWidget *parent)
{
	InitializeProfile();
	InitialiseDockElements();
	InitialiseDialog(parent);
}
