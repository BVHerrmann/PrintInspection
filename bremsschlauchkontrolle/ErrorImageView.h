#pragma once
#include <qgraphicsview.h>
#include "qgraphicsitem.h"
#include "ImageMetaData.h"


class MeasurWindowRectErrorText
{
public:
	MeasurWindowRectErrorText()
	{
		m_ErrorTextModelNotFound     = NULL;
		m_ErrorTextPrintNotOk        = NULL;
		m_ErrorTextPositionNotOk     = NULL;
		m_ErrorTextFormatLenghtNotOk = NULL;
		m_ErrorTextBlockLenghtNotOk  = NULL;
		m_ErrorTextBlockHeightNotOk  = NULL;
		m_ErrorTextFormatNotFound    = NULL;
		m_ErrorTextLineMissing       = NULL;
	}
public:
	QGraphicsTextItem     *m_ErrorTextModelNotFound, *m_ErrorTextPrintNotOk, *m_ErrorTextPositionNotOk, *m_ErrorTextFormatLenghtNotOk, *m_ErrorTextBlockLenghtNotOk, *m_ErrorTextBlockHeightNotOk,  *m_ErrorTextFormatNotFound, *m_ErrorTextLineMissing;
};


class ImageData;
class ErrorImageView :	public QGraphicsView
{
	Q_OBJECT
public:
	ErrorImageView(ImageData *pImageData);
	~ErrorImageView();
	void DrawErrorInfo(const ImageMetaData &Image);
	ImageData  *GetImageData() { return m_ImageData; }
	void ClearGraphicItem();
	void ClearGraphicItem(QGraphicsItem *Item);
	void SetHTMLGraphicTextItem(QGraphicsTextItem *Item, QString &text, int xpos, int ypos, Qt::GlobalColor Color);
	void GetInspectionWindowFormatRectXY(const ImageMetaData &Image, double &XLeftTopPos, double &YLeftTopPos);
	void SetMeasurWindowRectErrorTextItems();
	void ClearAllMeasurWindowRectErrorTextItems();
	void ZoomRect(QRectF &rect, double ZoomFactor);
	void ClearImage();

public slots:
	void SlotShowErrorImage(const ImageMetaData &Image);

private:
	QGraphicsTextItem               *m_ErrorTextHeadLine;
	QGraphicsScene                  *m_GrapicSenceLiveImage;
	QGraphicsRectItem               *m_MeasureWindowItemFormatRect;
	QGraphicsPixmapItem             *m_Pixmap;
	ImageData                       *m_ImageData;
	QList<MeasurWindowRectErrorText> m_ListMeasurWindowRectErrorText;
};

