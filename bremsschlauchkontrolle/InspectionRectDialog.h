#pragma once
#include <QtWidgets>
#include <qdialog.h>
#include "ui_InspectionRectDialog.h"
#include "popupdialog.h"


class ImageData;
class InspectionRectDialog :	public QDialog
{
	Q_OBJECT
public:
	InspectionRectDialog(ImageData *pImageData,QWidget *parent=NULL);
	~InspectionRectDialog();
	ImageData *GetImageData() { return m_ImageData; }
	void ShowLineCheckParameter(bool show);
	void SetInspectionRectID(int iD, bool DeleteButtonOn=false);
	bool IsDeleteButtonOn() { return m_DeleteButtonOn; }

public slots:
	void SlotOKButtonIsClicked();
	void SlotDeleteCliced();
	void SlotLineCheckChanged(int);
	void SlotEnableBlockChanged(int);

private:
	Ui::InspectionRectDialog  ui;
	ImageData                *m_ImageData;
	int                       m_CurrentInspectionRectIndex;
	bool                      m_DeleteButtonOn;
	bool                      m_SetupWindow;
};


class PopupDialogInspectionRectDialog : public PopupDialog
{

public:
	PopupDialogInspectionRectDialog(ImageData *pImageData, QWidget *parent);
    void SetInspectionRectID(int iD, bool DeleteButtonOn, const QString &CancelText, const QString &ApplyText);

public:
	InspectionRectDialog *m_Dialog;
	QDialogButtonBox     *m_button_box;
};

