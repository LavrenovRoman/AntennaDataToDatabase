#include "antennadatatodatabase.h"
#include <QFileDialog>

AntennaDataToDatabase::AntennaDataToDatabase(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//::setCodecForTr(QTextCodec::codecForName("Windows-1251"));
	connect(ui.but_OpenDir, SIGNAL(clicked()), this, SLOT(ClickedOpenDir()));
	connect(ui.but_LoadOutToDB, SIGNAL(clicked()), this, SLOT(ClickedOpenOutFiles()));
	
	ui.lbl_FindFiles->setText("");
	ui.but_OpenDir->setEnabled(true);
	ui.but_LoadOutToDB->setEnabled(false);

	int res = core.ConnectDatabase();
	
	if (res == -1)
	{
		ui.lbl_FindFiles->setText(QString::fromLocal8Bit("Ошибка! Не найден файл Options.ini!"));
		ui.but_OpenDir->setEnabled(false);
	}
	else
	{
		if (res == -2)
		{
			ui.lbl_FindFiles->setText(QString::fromLocal8Bit("Ошибка! Не могу подключиться к БД!"));
			ui.but_OpenDir->setEnabled(false);
		}
	}
}

AntennaDataToDatabase::~AntennaDataToDatabase()
{

}

void AntennaDataToDatabase::ClickedOpenDir()
{
	QString strdir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Открыть папку"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	int cntOutFiles = 0;
	int cntPreFiles = 0;

	int res = core.OpenDirectory(strdir, cntOutFiles, cntPreFiles);
	ui.lbl_FindFiles->setText(QString::fromLocal8Bit("Найдено out файлов: ") + QString::number(cntOutFiles) + QString::fromLocal8Bit("\n") + QString::fromLocal8Bit("Найдено pre файлов: ") + QString::number(cntPreFiles));
	ui.lbl_Result->setText(QString::fromLocal8Bit(""));

	if (res == 0) 
	ui.but_LoadOutToDB->setEnabled(true);
	else
	ui.but_LoadOutToDB->setEnabled(false);
}

void AntennaDataToDatabase::ClickedOpenOutFiles()
{
	if (core.ReadFiles() == 0)
	{
		ui.lbl_Result->setText(QString::fromLocal8Bit("Файлы считаны"));
		ui.lbl_Result->repaint();
	}
	if (core.PrepareExperimentBeforeWrite() == 0 && core.WriteData() == 0)
	{
		ui.lbl_Result->setText(QString::fromLocal8Bit("Записано в БД"));
		ui.lbl_Result->repaint();
	}
}