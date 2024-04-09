#pragma once
#include <QtWidgets>
#include <qDialog.h>
#include "popupdialog.h"
#include "ui_WidgetEditCameraROI.h"

class MainAppPrintCheck;
class WidgetEditCameraROI :	public QWidget
{
public:
	WidgetEditCameraROI(MainAppPrintCheck *pMainAppPrintCheck , int CameraIndex);
	~WidgetEditCameraROI();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	void showEvent(QShowEvent *event);

public slots:
	void SlotApplySettings();
	void SlotCancel();

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	int m_CameraIndex;
	Ui::WidgetEditCameraROI ui;
};


class PopupDialogEditCameraROI : public PopupDialog
{
	
public:
	PopupDialogEditCameraROI(MainAppPrintCheck *pMainAppPrintCheck, int CameraIndex, QWidget *parent);
	void SetButtonText(QString &ApplyText, QString &CancelText);

public:
	QDialogButtonBox     *m_button_box;
	
};

