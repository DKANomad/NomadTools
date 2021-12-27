#pragma once

#include <QtWidgets/qdockwidget.h>

class MainDock : public QDockWidget {
public:
	inline MainDock(QWidget *parent = nullptr) : QDockWidget(parent) {}
	void on_groupRecordingsButton_clicked();
};
