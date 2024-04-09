#pragma once
#include <qwidget.h>
#include "ui_WidgetEditMeasureParameter.h"

class MainAppPrintCheck;
class WidgetEditMeasureParameter :	public QWidget
{
public:
	WidgetEditMeasureParameter(MainAppPrintCheck *pParent, int CameraIndex);
	~WidgetEditMeasureParameter();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	void showEvent(QShowEvent *event);

public slots:
	void SlotApplyParameter();

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	int m_CameraIndex;
	Ui::WidgetEditMeasureParameter ui;
};

