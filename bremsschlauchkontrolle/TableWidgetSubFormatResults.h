#pragma once
#include <qtableview.h>
#include "qstandarditemmodel.h"

class ImageData;
class TableWidgetSubFormatResults :	public QTableView
{
public:
	TableWidgetSubFormatResults(ImageData *pImageData);
	~TableWidgetSubFormatResults();
	ImageData *GetImageData() { return m_ImageData; }
	void UpdateResults();

private:
	QStandardItemModel *m_ItemModel;
	ImageData *m_ImageData;
};

