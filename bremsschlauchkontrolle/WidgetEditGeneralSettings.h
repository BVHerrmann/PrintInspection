#pragma once
#include <qwidget.h>
#include "ui_WidgetEditGeneralSettings.h"


class MainAppPrintCheck;
class WidgetEditGeneralSettings :	public QWidget
{
public:
	WidgetEditGeneralSettings(MainAppPrintCheck *pParent);
	~WidgetEditGeneralSettings();
	void showEvent(QShowEvent *event);
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	void ShowCalculatetPixelSize(double PixelSize, double Diameter, int CameraIndex);

public slots:
	void SlotApplyGeneralSettings();
	void SlotApplyPixelSizeTop();
	void SlotApplyPixelSizeBot();

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	Ui::WidgetEditGeneralSettings ui;
};

