#ifndef WIDGETCOPYDB_H
#define WIDGETCOPYDB_H

#include <QtWidgets/QMainWindow>
#include "ui_widgetcopydb.h"
#include "selectall.h"

class WidgetCopyDB : public QDialog
{
	Q_OBJECT

public:
	WidgetCopyDB(SelectAll* pCoreData, Core* pCore, QWidget *parent = 0);
	~WidgetCopyDB();
	void Reset();

signals:
	void Cancel();

private:
	Core * pkCore = nullptr;
	SelectAll * pkCoreData = nullptr;
	Ui::DialogCopyDB ui;
	QString pathDB1, pathDB2;
	std::string dirDB;

private slots:
	void SelectDB1();
	void SelectDB2();
	void CopyDB();
};

#endif // WIDGETCOPYDB_H