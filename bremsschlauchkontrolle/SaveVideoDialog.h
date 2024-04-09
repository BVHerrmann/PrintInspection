#pragma once

#include <QDialog>
#include "ui_SaveVideoDialog.h"


class SaveVideoDialog : public QDialog
{
	Q_OBJECT

public:
	SaveVideoDialog();
	~SaveVideoDialog();
	void showEvent(QShowEvent *);
	QString GetFileName();
	QString GetLocation();
	void SetFileLocation(QString &FileLocation);
	

public slots:
	void SlotOkPressed();
	void SlotCancelPressed();

private:
	Ui::SaveVideoDialog ui;
	
};
