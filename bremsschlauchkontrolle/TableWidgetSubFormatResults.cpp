#include "TableWidgetSubFormatResults.h"
#include "ImageData.h"
#include "InspectionWindow.h"
#include "qheaderview.h"
#include "GlobalConst.h"


TableWidgetSubFormatResults::TableWidgetSubFormatResults(ImageData *pImageData) : QTableView(NULL)
, m_ItemModel(NULL)
{
	m_ImageData = pImageData;
	m_ItemModel = new QStandardItemModel();
	QStringList ListHeaderData = GetImageData()->GetHeaderListInspectionResults();
	// Set Model Headers
    m_ItemModel->setHorizontalHeaderLabels(ListHeaderData);
	horizontalHeader()->setDefaultSectionSize(100);
	setModel(m_ItemModel);
}


TableWidgetSubFormatResults::~TableWidgetSubFormatResults()
{
}

void TableWidgetSubFormatResults::UpdateResults()
{
	ListResultsInspectionWindows *pListResultsSubFormat = GetImageData()->GetListResultsInspectionWindows();
	QList< QStandardItem *> ListItems;
    QHashIterator<int, InspectionWindow> i(*pListResultsSubFormat);
	QStringList ListResults;
	InspectionWindow pInspectionWindow;
	int row = 0;
	int InspectionWindowIDFormat = 0;
	

	m_ItemModel->removeRows(0, m_ItemModel->rowCount());


	//Format dat First
	pInspectionWindow=pListResultsSubFormat->value(InspectionWindowIDFormat);
	GetImageData()->GetListMeasurementResults(&pInspectionWindow, ListResults);
	for (int k = 0; k < ListResults.count(); k++)
		ListItems.append(new QStandardItem(ListResults.at(k)));
	m_ItemModel->insertRow(row, ListItems);
	ListItems.clear();
	row++;
	m_ItemModel->insertRow(row, ListItems);//eine leere Reihe einfügen
	row++;

	while (i.hasNext())
	{
		i.next();
		pInspectionWindow = i.value();
		
		if (pInspectionWindow.m_InspectionWindowID != INSPECTION_ID_FORMAT_WINDOW && pInspectionWindow.m_InspectionWindowID != INSPECTION_ID_HOSE_WINDOW)
		{
			GetImageData()->GetListMeasurementResults(&pInspectionWindow, ListResults);
			for (int k = 0; k < ListResults.count(); k++)
				ListItems.append(new QStandardItem(ListResults.at(k)));
			m_ItemModel->insertRow(row, ListItems);
			ListItems.clear();
			row++;
		}
	}
}
