#pragma once
#include <qgraphicsview.h>
#include "ImageMetaData.h"

class MeasurWindowRectGraphicsItem
{
public:
	MeasurWindowRectGraphicsItem()
	{
		m_MeasureWindowItem       = NULL;
		m_MeasureWindowItemRight  = NULL;
		m_MeasureWindowItemLeft   = NULL;
		m_MeasureWindowItemBot    = NULL;
		m_MeasureWindowItemTop    = NULL;
		m_MeasureWindowResultText = NULL;
		m_MeasureWindowItemROI    = NULL;
	}
	
public:
	QGraphicsRectItem     *m_MeasureWindowItem, *m_MeasureWindowItemRight, *m_MeasureWindowItemLeft, *m_MeasureWindowItemBot, *m_MeasureWindowItemTop,*m_MeasureWindowItemROI;
	QGraphicsTextItem     *m_MeasureWindowResultText;
};

class PopupDialogInspectionRectDialog;
class ImageData;//Parent Class
class LiveImageView : public QGraphicsView
{
	Q_OBJECT
    public:
	  LiveImageView(ImageData *pParent,bool IsLiveImage);
	   ~LiveImageView();
	   ImageData              *GetImageData()            { return m_ImageData; }
	   PopupDialogInspectionRectDialog   *GetPopupDialogInspectionRectDialog() { return m_PopupDialogInspectionRectDialog; }
	   void showEvent(QShowEvent *event);
	   void DrawAllMeasureResults(const ImageMetaData &Image);
	   void DrawAllReferenceRects();
	   void mousePressEvent(QMouseEvent * e);
	   void mouseMoveEvent(QMouseEvent * e);
	   void mouseReleaseEvent(QMouseEvent * e);
	   void ClearGraphicItem(QGraphicsItem *Item);
	   void GetRectSupportPoints(QRectF &CurrentRect, QRectF &TopSquareMiddelPos, QRectF &BottomSquareMiddelPos, QRectF &LeftSquareMiddelPos, QRectF &RightSquareMiddelPos);
	   void MoveAndResizeMeasureWindow(int NewXPos, int NewYPos, int CurserPosition, int InspectionWindowID, double Zoom);
	   void ZoomRect(QRectF &rect, double ZoomFactor);
	   void SetCurserForMeasureWindowROI(int x, int y);
	   void MoveROIRect(QRectF &ROIRect, int NewXPos, int NewYPos, int InspectionWindowID, double Zoom);
	   void SetHTMLGraphicTextItem(QGraphicsTextItem *Item, QString &text, int xpos, int ypos, Qt::GlobalColor Color= Qt::white);
	   int  ShowInspectionDialog(int CurrentInspectionWindowIndex,bool DeleteButton=false);
	   void DeleteSelectedRect();
	   void SetMeasureWindowRectGraphicsItem();
	   QRectF GetSelectedRect();
	   void ClearAllMeasureWindowGraphicsItems();
	   void DrawFormatRect();
	   void ShowHoseNotFoundText();
	   void ClearTextHoseNotFound();
	   void SetMeasureRulerIsActive(bool Active);
	   void DrawMeasureLines();
	   void GetLineSupportPoint(QLine &lineX, QLine &lineY,QRectF &LineRectPoint);
	   void SetCurserForMeasureRuler(int x, int y);
	   void ClearAllMeasureRulerItems();
	   void YCenterAllMeasureWindow();
	   void GetInspectionWindowFormatRectXY(const ImageMetaData &Image, double &XLeftTopPos, double &YLeftTopPos);
	
private:
	   void DrawReferenceRect(int InsepectionWindowIndex);
	   
public slots:
	void SlotShowLiveImage(const ImageMetaData &Image);
	void SlotSelectionChanged();

private:
	QGraphicsScene                     *m_GrapicSenceLiveImage;
	QGraphicsPixmapItem                *m_Pixmap;
	QGraphicsLineItem                  *m_HorizontalLineTopEdge, *m_HorizontalLineBottomEdge;
	QGraphicsLineItem                  *m_LineItemHoseMiddlePos;
	QGraphicsTextItem                  *m_TextItemHoseNotFound;
	QGraphicsLineItem                  *m_LineItemLeftMeasureLine, *m_LineItemRightMeasureLine;
	QGraphicsLineItem                  *m_LineItemTopMeasureLine, *m_LineItemBotMeasureLine;
	QGraphicsEllipseItem               *m_RectItemLeftMeasureLine, *m_RectItemRightMeasureLine;
	QGraphicsTextItem                  *m_TextItemHorMeasureDistance;
	ImageData                          *m_ImageData;
	bool                                m_IsFormatLiveImage;
	bool                                m_MousePressEventIsClicked;
	bool                                m_MeasureRulerIsActive;
	int                                 m_CurrentImageWidth;
	int                                 m_CurrentImageHeight;
	int                                 m_CurserPosition;
	int                                 m_CurserPositionRuler;
	int                                 m_CurrentInspectionWindowIndex;
	int                                 m_YposSupportPointMeasureRuler;
	QLine                               m_LeftMeasureLine, m_RightMeasureLine;
	QLine                               m_TopMeasureLine, m_BotMeasureLine;
	QPointF                             m_ClickPos;
	PopupDialogInspectionRectDialog    *m_PopupDialogInspectionRectDialog;
	QList<MeasurWindowRectGraphicsItem> m_ListMeasureWindowItems;
};

