#pragma once

#include "qimage.h"
#include "qpixmap.h"
#include "InspectionWindow.h"


class ImageMetaData
{
public:
	ImageMetaData() 
	{
	}
   
public:
	QImage m_Image;
	QPixmap m_Pixmap;
	double m_HoseMiddlePosition;
	QList<InspectionWindow>  m_ListInspectionWindowResults;
};
Q_DECLARE_METATYPE(ImageMetaData);