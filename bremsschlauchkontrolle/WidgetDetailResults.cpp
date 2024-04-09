#include "WidgetDetailResults.h"
#include "qtabbar.h"



WidgetDetailResults::WidgetDetailResults(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	for (int i = 0; i < ui.tabWidgetResults->count(); i++)
	{
		delete ui.tabWidgetResults->widget(i);
	}
	ui.tabWidgetResults->clear();

	ui.tabWidgetResults->tabBar()->setAutoHide(true);
}


WidgetDetailResults::~WidgetDetailResults()
{
}


void WidgetDetailResults::InsertNewTab(int index, QWidget *pWidget, QString &Label)
{
	ui.tabWidgetResults->insertTab(index, pWidget,Label);
}


QWidget *WidgetDetailResults::GetCurrentWidget()
{
	return ui.tabWidgetResults->currentWidget();
}

QWidget *WidgetDetailResults::GetWidget(int index)
{
	return ui.tabWidgetResults->widget(index);
}


