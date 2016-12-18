#include "wait_download.h"

using namespace std;

Wait_Download::Wait_Download(QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);
}

Wait_Download::~Wait_Download()
{

}

void Wait_Download::Reset()
{
	ui.progressBar->setValue(0);
}

void Wait_Download::WaitChanged(int waitChange)
{
	ui.progressBar->setValue(waitChange);
}