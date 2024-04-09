#pragma once
#include <qwidget.h>
#include "ui_WidgetDetailResults.h"

class WidgetDetailResults :	public QWidget
{
	Q_OBJECT
public:
	WidgetDetailResults(QWidget *parent);
	~WidgetDetailResults();
	void InsertNewTab(int index, QWidget *pWidhet, QString &Label);
	QWidget *GetCurrentWidget();
	QWidget *GetWidget(int index);

private:
	Ui::WidgetDetailResults ui;
};

