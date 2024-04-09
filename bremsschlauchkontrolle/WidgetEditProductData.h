#pragma once
#include <qwidget.h>
#include "ui_WidgetEditProductData.h"


class MainAppPrintCheck;
class WidgetEditProductData :	public QWidget
{
	Q_OBJECT
public:
	WidgetEditProductData(MainAppPrintCheck *pParent);
	~WidgetEditProductData();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	void showEvent(QShowEvent *event);

public slots:
	void SlotApplySettings();

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	Ui::WidgetEditProductData ui;
};

