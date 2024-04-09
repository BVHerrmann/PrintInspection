#include "SaveVideoDialog.h"
#include "qfileinfo.h"
#include "bmessagebox.h"


SaveVideoDialog::SaveVideoDialog() : QDialog(NULL)
{
	ui.setupUi(this);
	
	ui.lineEditInputDataVideoLocation->setReadOnly(true);
	connect(ui.pushButtonOK,     &QPushButton::clicked, this, &SaveVideoDialog::SlotOkPressed);
	connect(ui.pushButtonCancel, &QPushButton::clicked, this, &SaveVideoDialog::SlotCancelPressed);
}


SaveVideoDialog::~SaveVideoDialog()
{
}


void SaveVideoDialog::showEvent(QShowEvent *)
{
	ui.lineEditInputDataNewVideoFile->setText("Video1");
}


void SaveVideoDialog::SetFileLocation(QString &FileLocation)
{
	ui.lineEditInputDataVideoLocation->setText(FileLocation);
}


QString SaveVideoDialog::GetFileName()
{
  return ui.lineEditInputDataNewVideoFile->text();
}


QString SaveVideoDialog::GetLocation()
{
  return ui.lineEditInputDataVideoLocation->text();
}


void SaveVideoDialog::SlotOkPressed()
{
	QString Location    = ui.lineEditInputDataVideoLocation->text();
	QString FileName    = ui.lineEditInputDataNewVideoFile->text();
	QString PathAndName = Location + QString("/") + FileName + ".bmp";

	QFileInfo fileInfo(PathAndName);

	if (fileInfo.exists())
	{
		if(BMessageBox::information(NULL, tr("File exist"), tr("File exist, Overwrite ?"), QMessageBox::Yes| QMessageBox::No) == QDialog::Accepted)
			accept();
	}
	else
		accept();
}


void SaveVideoDialog::SlotCancelPressed()
{
	reject();
}

