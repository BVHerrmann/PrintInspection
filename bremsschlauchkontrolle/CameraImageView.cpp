#include "CameraImageView.h"
#include "qgraphicsitem.h"
#include "ImageData.h"


CameraImageView::CameraImageView(ImageData *pParent) : QGraphicsView()
, m_GrapicSenceLiveImage(NULL)
, m_Pixmap(NULL)
, m_ImageData(NULL)
, m_HorizontalLineTopEdge(NULL)
, m_HorizontalLineBottomEdge(NULL)
, m_ResultText(NULL)
, m_MeasureWindowItem(NULL)
{
	m_GrapicSenceLiveImage = new QGraphicsScene(this);
	m_Pixmap = m_GrapicSenceLiveImage->addPixmap(QPixmap());
	m_ImageData = pParent;
	setScene(m_GrapicSenceLiveImage);
	setMouseTracking(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


CameraImageView::~CameraImageView()
{
}

//Anzeige Aktuelles Kamerabild
void CameraImageView::SlotShowCameraImage(const ImageMetaData &Image)
{
	if (m_Pixmap)
	{
		m_Pixmap->setPixmap(QPixmap::fromImage(Image.m_Image));
		setSceneRect(m_Pixmap->boundingRect());
		DrawMeasureResults(Image);//Ergebnis der Schlauchkanten/Schlauchposition
	}
}

//Einzeichnen der Messergebnisse in das Overlay des Kamerabildes. Schlauchkante in grün Messfenster in blau und Textinfo
void CameraImageView::DrawMeasureResults(const ImageMetaData &Image)
{
	if (GetImageData())
	{
		int x1, y1, x2, y2;
		QLine LeftEdge;
		QLine RightEdge;
		QRectF CurrentROIRect;
		QPen PenColor(Qt::green,2);
		QPen PenColorBlue(Qt::blue);
		QString TextMeasuringResults;
		double DisplayZoomFactor = GetImageData()->GetDisplayZoomFactorCameraImage();
		double PixelSize = GetImageData()->GetPixelSize();
		QPointF origin;


		origin.setX((GetImageData()->GetCameraWidthInPixel()/2.0 ) *  DisplayZoomFactor);
	    origin.setY((GetImageData()->GetCameraHeightInPixel()/2.0) *  DisplayZoomFactor);
		ClearGraphicItem(m_LeftEdge);
		ClearGraphicItem(m_RightEdge);
		ClearGraphicItem(m_MeasureWindowItem);
		ClearGraphicItem(m_ResultText);
		if (Image.m_ListInspectionWindowResults.count()>0)
		{
			if (Image.m_ListInspectionWindowResults[0].m_Results.m_ModelFound)
			{
				x1 = Image.m_ListInspectionWindowResults[0].m_Results.m_ResultXPos - Image.m_ListInspectionWindowResults[0].m_Results.m_ObjectSizeInX / 2.0;
				y1 = 0;
				x2 = x1;
				y2 = Image.m_ListInspectionWindowResults[0].m_Results.m_ObjectSizeInY - 1;
				x1 = static_cast<int>(x1 * DisplayZoomFactor + 0.5);
				x2 = static_cast<int>(x2 * DisplayZoomFactor + 0.5);
				y1 = static_cast<int>(y1 * DisplayZoomFactor + 0.5);
				y2 = static_cast<int>(y2 * DisplayZoomFactor + 0.5);
				LeftEdge.setLine(x1, y1, x2, y2);//Position linke Kante
				
				x1 = Image.m_ListInspectionWindowResults[0].m_Results.m_ResultXPos + Image.m_ListInspectionWindowResults[0].m_Results.m_ObjectSizeInX / 2.0;
				y1 = 0;
				x2 = x1;
				y2 = Image.m_ListInspectionWindowResults[0].m_Results.m_ObjectSizeInY - 1;
				x1 = static_cast<int>(x1 * DisplayZoomFactor + 0.5);
				x2 = static_cast<int>(x2 * DisplayZoomFactor + 0.5);
				y1 = static_cast<int>(y1 * DisplayZoomFactor + 0.5);
				y2 = static_cast<int>(y2 * DisplayZoomFactor + 0.5);
				RightEdge.setLine(x1, y1, x2, y2);//Position rechte Kante

				m_LeftEdge  = m_GrapicSenceLiveImage->addLine(LeftEdge, PenColor);
				m_LeftEdge->setTransformOriginPoint(origin);
				m_LeftEdge->setRotation(-90.0);//Da Bild in der Anzeige um 90 grad gedreht
				m_RightEdge = m_GrapicSenceLiveImage->addLine(RightEdge, PenColor);
				m_RightEdge->setTransformOriginPoint(origin);
				m_RightEdge->setRotation(-90.0);//Da Bild in der Anzeige um 90 grad gedreht
				TextMeasuringResults = tr("<>M:%1%</><br>D:%2mm</br>").arg(Image.m_ListInspectionWindowResults[0].m_Results.m_ModelScore, 0, 'f', 1).arg(Image.m_ListInspectionWindowResults[0].m_Results.m_ObjectSizeInX*PixelSize, 0, 'f', 1);
				m_ResultText = m_GrapicSenceLiveImage->addText("");
				SetHTMLGraphicTextItem(m_ResultText, TextMeasuringResults, 0, 0);
			}
			else
			{
				TextMeasuringResults  = tr("<>%1</>").arg(tr("Hose Not Found"));
				m_ResultText          = m_GrapicSenceLiveImage->addText("");
				SetHTMLGraphicTextItem(m_ResultText, TextMeasuringResults, 0, 0);
			}
			CurrentROIRect = Image.m_ListInspectionWindowResults[0].m_ReferenceRect;
			if (DisplayZoomFactor != 1.0)
				ZoomRect(CurrentROIRect, DisplayZoomFactor);
			CurrentROIRect = QRect(CurrentROIRect.y(), CurrentROIRect.x(), CurrentROIRect.height(), CurrentROIRect.width());//Da Bild in der Anzeige um 90 grad gedreht
			m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentROIRect, PenColorBlue);
		}
	}
}


void CameraImageView::ZoomRect(QRectF &rect, double ZoomFactor)
{
	QPointF TopLeft, BottomRight;
	TopLeft     = rect.topLeft()     * ZoomFactor;
	BottomRight = rect.bottomRight() * ZoomFactor;
	rect.setTopLeft(TopLeft);
	rect.setBottomRight(BottomRight);
}


void CameraImageView::SetHTMLGraphicTextItem(QGraphicsTextItem *Item, QString &text, int xpos, int ypos)
{
	Item->setHtml(QString("<p style='background:rgba(120,120,120, 100%);'>" + text + QString("</p>")));
	Item->setDefaultTextColor(Qt::white);
	Item->setPos(xpos, ypos);
}


void CameraImageView::ClearGraphicItem(QGraphicsItem *Item)
{
	QList<QGraphicsItem *> items = m_GrapicSenceLiveImage->items();
	for (int i = 0; i < items.size(); i++)
	{
		QGraphicsItem *item = items.at(i);
		if (item == Item)
		{
			m_GrapicSenceLiveImage->removeItem(item);
			delete item;
			break;
		}
	}
}
