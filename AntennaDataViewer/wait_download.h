#ifndef WAIT_DOWNLOAD_H
#define WAIT_DOWNLOAD_H

#include <QtWidgets/QMainWindow>
#include "QThread"
#include "ui_wait_download.h"

class Wait_Download : public QDialog
{
	Q_OBJECT

public:
	Wait_Download(QWidget *parent = 0);
	~Wait_Download();
	void Reset();
	void WaitChanged(int waitChange);

private:
	Ui::Dialog_Wait_Download ui;
};

#endif // WAIT_DOWNLOAD_H