#pragma once
#include <qwidget.h>
#include "ui_WidgetEditCustomSettings.h"

class MainAppPrintCheck;
class WidgetEditCustomSettings :	public QWidget
{
public:
	WidgetEditCustomSettings(MainAppPrintCheck *pParent);
	~WidgetEditCustomSettings();
	void showEvent(QShowEvent *event);
	MainAppPrintCheck *GetMainAppPrintCheck() {	return m_MainAppPrintCheck;}

public slots:
	void SlotApplySettings();
	

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	Ui::WidgetEditCustomSettings ui;
};

