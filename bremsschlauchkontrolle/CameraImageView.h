#pragma once
#include <qgraphicsview.h>
#include "ImageMetaData.h"

class ImageData;
class CameraImageView :	public QGraphicsView
{
public:
	CameraImageView(ImageData *pParent);
	~CameraImageView();
	ImageData              *GetImageData() { return m_ImageData; }
	void                    DrawMeasureResults(const ImageMetaData &Image);
	void                    ClearGraphicItem(QGraphicsItem *Item);
	void                    SetHTMLGraphicTextItem(QGraphicsTextItem *Item, QString &text, int xpos, int ypos);
	void                    ZoomRect(QRectF &rect, double ZoomFactor);
	

public slots:
	void CameraImageView::SlotShowCameraImage(const ImageMetaData &Image);

private:
	QGraphicsScene        *m_GrapicSenceLiveImage;
	QGraphicsPixmapItem   *m_Pixmap;
	QGraphicsLineItem     *m_HorizontalLineTopEdge, *m_HorizontalLineBottomEdge;
	QGraphicsLineItem     *m_LeftEdge, *m_RightEdge;
	QGraphicsTextItem     *m_ResultText;
	QGraphicsRectItem     *m_MeasureWindowItem;
	ImageData             *m_ImageData;
};

