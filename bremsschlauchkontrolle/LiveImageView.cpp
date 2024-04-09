#include "LiveImageView.h"
#include "qgraphicsitem.h"
#include "qevent.h"
#include "ImageData.h"
#include "GlobalConst.h"
#include "ProductData.h"
#include "InspectionWindow.h"
#include "InspectionRectDialog.h"
#include "InspectionWindow.h"
#include <qapplication.h>
#include "qscrollbar.h"
#include "MainAppPrintCheck.h"


LiveImageView::LiveImageView(ImageData *pParent,bool IsLiveImage) : QGraphicsView()
,m_GrapicSenceLiveImage(NULL)
,m_Pixmap(NULL)
,m_ImageData(NULL)
,m_HorizontalLineTopEdge(NULL)
,m_HorizontalLineBottomEdge(NULL)
,m_MousePressEventIsClicked(false)
,m_CurserPosition(CURSER_POSITION_NOT_ON_ROI)
,m_CurserPositionRuler(CURSER_POSITION_NOT_ON_ROI)
,m_CurrentImageWidth(0)
,m_CurrentImageHeight(0)
,m_CurrentInspectionWindowIndex(0)
,m_IsFormatLiveImage(true)
,m_PopupDialogInspectionRectDialog(NULL)
,m_LineItemHoseMiddlePos(NULL)
,m_TextItemHoseNotFound(NULL)
,m_MeasureRulerIsActive(false)
,m_LineItemLeftMeasureLine(NULL)
,m_LineItemRightMeasureLine(NULL)
,m_LineItemTopMeasureLine(NULL)
,m_LineItemBotMeasureLine(NULL)
,m_RectItemLeftMeasureLine(NULL)
,m_RectItemRightMeasureLine(NULL)
,m_TextItemHorMeasureDistance(NULL)
,m_YposSupportPointMeasureRuler(250)
{
	m_GrapicSenceLiveImage       = new QGraphicsScene(this);
	m_Pixmap                     = m_GrapicSenceLiveImage->addPixmap(QPixmap());
	m_ImageData                  = pParent;
	m_IsFormatLiveImage          = IsLiveImage;
	setScene(m_GrapicSenceLiveImage);
	setAlignment(Qt::AlignLeft| Qt::AlignTop);
	setMouseTracking(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	connect(m_GrapicSenceLiveImage, &QGraphicsScene::selectionChanged, this,&LiveImageView::SlotSelectionChanged);
}


LiveImageView::~LiveImageView()
{
}


void LiveImageView::SlotSelectionChanged()
{
	QList<QGraphicsItem *> ListSelected = m_GrapicSenceLiveImage->selectedItems();
	if (ListSelected.count() > 0)
	{
		QGraphicsItem *pMeasureWindowItem = ListSelected.at(0);
		if (pMeasureWindowItem)
		{
			for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
			{
				InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
				if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
				{
					if (m_ListMeasureWindowItems[i].m_MeasureWindowItem == (QGraphicsRectItem *)(pMeasureWindowItem))
					{
						QRectF CurrentRect = pInspectionWindow->m_ReferenceRect;
						GetImageData()->ShowSelectedRectKoordinates(CurrentRect);
						break;
					}
				}
			}
		}
	}
}


QRectF LiveImageView::GetSelectedRect()
{
	QRectF rect;
	QList<QGraphicsItem *> ListSelected = m_GrapicSenceLiveImage->selectedItems();
	if (ListSelected.count() > 0)
	{
		QGraphicsItem *pMeasureWindowItem = ListSelected.at(0);//es ist Maximal immer nur ein Rechteck ausgewählt
		if (pMeasureWindowItem)
		{
			for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++) 
			{
				InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
				if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
				{
					if (m_ListMeasureWindowItems[i].m_MeasureWindowItem == (QGraphicsRectItem *)(pMeasureWindowItem))
					{
						rect = pInspectionWindow->m_ReferenceRect;
						break;
					}
				}
			}
		}
	}
	return rect;
}


void LiveImageView::SlotShowLiveImage(const ImageMetaData &Image)
{
	if (m_Pixmap)
	{
		m_CurrentImageWidth  = Image.m_Image.width();
		m_CurrentImageHeight = Image.m_Image.height();
		
		m_Pixmap->setPixmap(QPixmap::fromImage(Image.m_Image));
		QRectF Brect = m_Pixmap->boundingRect();
		
		if (m_IsFormatLiveImage)
		{
			double NewYPos = (rect().height() - Brect.height()) / 2.0;
			if (NewYPos > 0.0)//mittig Horizontal positionieren
			{
				Brect.setY(NewYPos*-1.0);
				setSceneRect(Brect);
			}
			
			if (GetImageData()->GetSaveErrorImagePoolCondition() == SAVE_FORMAT_IMAGE_ONLY_GOOD_IMAGES || GetImageData()->GetSaveErrorImagePoolCondition() == SAVE_FORMAT_IMAGE_ALL_IMAGES)
			{
				ImageMetaData ResultsAndImage;

				ResultsAndImage.m_Pixmap = grab();//Kopiere Fensterinhalt
				for (int i = 0; i < Image.m_ListInspectionWindowResults.count(); i++)
				{
					InspectionWindow InspectionResults = Image.m_ListInspectionWindowResults.at(i);
					ResultsAndImage.m_ListInspectionWindowResults.append(InspectionResults);
				}
				GetImageData()->AppendInspectionResultsForStorage(ResultsAndImage);
			}
		}
		else
		{
			int center;
			verticalScrollBar()->setMaximum(m_CurrentImageHeight / 2.0);
			center = (verticalScrollBar()->minimum() + verticalScrollBar()->maximum()) / 2;
			verticalScrollBar()->setValue(center);
			
			horizontalScrollBar()->setMaximum(m_CurrentImageWidth / 2.0);
			center = (horizontalScrollBar()->minimum() + horizontalScrollBar()->maximum()) / 2;
			horizontalScrollBar()->setValue(center);
			setSceneRect(Brect);
	    }
		SetMeasureWindowRectGraphicsItem();
		if (!m_IsFormatLiveImage)
		{
			DrawAllReferenceRects();
		}
		else
		{
			DrawAllMeasureResults(Image);
			if (GetImageData())
			{
				GetImageData()->ShowImageCounter(GetImageData()->GetFormatCounter());
				GetImageData()->AppendInspectionResultsForResultView(Image);
			}
		}
    }
}


void LiveImageView::ShowHoseNotFoundText()
{
	QFont Font("Times", 80, QFont::Bold);

	ClearAllMeasureWindowGraphicsItems();
	m_Pixmap->setPixmap(QPixmap());
	Font.setBold(true);
	ClearGraphicItem(m_TextItemHoseNotFound);
	m_TextItemHoseNotFound = m_GrapicSenceLiveImage->addText("");
	m_TextItemHoseNotFound->setHtml(QString("<p style='background:rgba(255,255,255, 100%);'>" + tr("No Production!") + QString("</p>")));
	m_TextItemHoseNotFound->setDefaultTextColor(Qt::blue);
	m_TextItemHoseNotFound->setFont(Font);
}


void LiveImageView::ClearTextHoseNotFound()
{
	ClearGraphicItem(m_TextItemHoseNotFound);
	m_TextItemHoseNotFound = NULL;
}


void LiveImageView::showEvent(QShowEvent *event)
{
	m_MeasureRulerIsActive = false;
	ClearAllMeasureRulerItems();
}


void LiveImageView::SetMeasureWindowRectGraphicsItem()
{
	int NumRects = GetImageData()->GetNumberInspectionWindows();

	ClearAllMeasureWindowGraphicsItems();
	if (NumRects != m_ListMeasureWindowItems.count())
	{
		m_ListMeasureWindowItems.clear();
		for (int i = 0; i < NumRects; i++)
		{
			MeasurWindowRectGraphicsItem NewItem;
			m_ListMeasureWindowItems.append(NewItem);
		}
	}
}


void LiveImageView::ClearAllMeasureWindowGraphicsItems()
{
	for (int i = 0; i < m_ListMeasureWindowItems.count(); i++)
	{
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItem);
		m_ListMeasureWindowItems[i].m_MeasureWindowItem = NULL;
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItemBot);
		m_ListMeasureWindowItems[i].m_MeasureWindowItemBot = NULL;
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItemLeft);
		m_ListMeasureWindowItems[i].m_MeasureWindowItemLeft = NULL;
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItemRight);
		m_ListMeasureWindowItems[i].m_MeasureWindowItemRight = NULL;
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItemTop);
		m_ListMeasureWindowItems[i].m_MeasureWindowItemTop = NULL;
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowResultText);
		m_ListMeasureWindowItems[i].m_MeasureWindowResultText = NULL;
		ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItemROI);
		m_ListMeasureWindowItems[i].m_MeasureWindowItemROI = NULL;
	}
}


void LiveImageView::DrawAllMeasureResults(const ImageMetaData &Image)
{
	if (GetImageData())
	{
		QString TextMeasuringResults;
		QPen PenColorGreen(Qt::green);
		QPen PenColorRed(Qt::red);
		QPen PenColorBlue(Qt::blue);
		double ZoomFactor       = GetImageData()->GetDisplayZoomFactorFormatImage();
		//double ErrorThreshold   = GetImageData()->GetCurrentProductData()->GetPrintErrorTolInPercent();
		int TextXpos;// , TextYpos = 10;
		QLine LineHoseMiddlePos;
		double HoseMiddlePosition = Image.m_HoseMiddlePosition * ZoomFactor;
		double HoseDiameter= GetImageData()->GetHoseDiameterInPixel() * ZoomFactor;
		double PrintYpos = HoseMiddlePosition + HoseDiameter / 2.0;
		

		LineHoseMiddlePos.setLine(0,static_cast<int>(HoseMiddlePosition + 0.5), rect().width()-1, static_cast<int>(HoseMiddlePosition + 0.5));
		for (int i = 0; i < Image.m_ListInspectionWindowResults.count(); i++)
		{
			QRectF CurrentRect;

			ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowItem);
			m_ListMeasureWindowItems[i].m_MeasureWindowItem = NULL;
			ClearGraphicItem(m_ListMeasureWindowItems[i].m_MeasureWindowResultText);
			m_ListMeasureWindowItems[i].m_MeasureWindowResultText = NULL;

			if (Image.m_ListInspectionWindowResults.at(i).m_Results.m_ModelFound)
			{
				QPointF TopLeft(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ResultXPos - Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInX / 2.0, Image.m_ListInspectionWindowResults.at(i).m_Results.m_ResultYPos - Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInY / 2.0);
				QPointF BotRight(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ResultXPos + Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInX / 2.0, Image.m_ListInspectionWindowResults.at(i).m_Results.m_ResultYPos + Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInY / 2.0);

				CurrentRect.setTopLeft(TopLeft);
				CurrentRect.setBottomRight(BotRight);
				if (ZoomFactor != 1.0)
					ZoomRect(CurrentRect, ZoomFactor);

				if (Image.m_ListInspectionWindowResults.at(i).m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
				{
					if (!Image.m_ListInspectionWindowResults.at(i).m_CheckOnlyHorizontalLines)
					{
						if (Image.m_ListInspectionWindowResults.at(i).m_Results.m_DefectScore > Image.m_ListInspectionWindowResults.at(i).m_PrintErrorTolInPercent)//ErrorThreshold)
							m_ListMeasureWindowItems[i].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect.toRect(), PenColorRed);
						else
							m_ListMeasureWindowItems[i].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect.toRect(), PenColorGreen);
						m_ListMeasureWindowItems[i].m_MeasureWindowItem->setTransformOriginPoint(m_ListMeasureWindowItems[i].m_MeasureWindowItem->rect().center());
						m_ListMeasureWindowItems[i].m_MeasureWindowItem->setRotation(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ModelAngle*(-1.0));

						TextMeasuringResults = tr("<>M:%1%</><br>%2E:%3%</br>").arg(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ModelScore, 0, 'f', 1).arg(QChar(916)).arg(Image.m_ListInspectionWindowResults.at(i).m_Results.m_DefectScore, 0, 'f', 1);
						m_ListMeasureWindowItems[i].m_MeasureWindowResultText = m_GrapicSenceLiveImage->addText("");
						SetHTMLGraphicTextItem(m_ListMeasureWindowItems[i].m_MeasureWindowResultText, TextMeasuringResults, CurrentRect.x(), PrintYpos);
					}
					else
					{
						if (Image.m_ListInspectionWindowResults.at(i).m_Results.m_LineCheckOk)
						{
							m_ListMeasureWindowItems[i].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect.toRect(), PenColorGreen);
						}
						else
						{
							m_ListMeasureWindowItems[i].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect.toRect(), PenColorRed);
						}
						TextMeasuringResults = tr("<>M:%1%</><br>%2E:%3%</br>").arg(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ModelScore, 0, 'f', 1).arg(QChar(916)).arg(Image.m_ListInspectionWindowResults.at(i).m_Results.m_DefectScore, 0, 'f', 1);
						m_ListMeasureWindowItems[i].m_MeasureWindowResultText = m_GrapicSenceLiveImage->addText("");
						SetHTMLGraphicTextItem(m_ListMeasureWindowItems[i].m_MeasureWindowResultText, TextMeasuringResults, CurrentRect.x(), PrintYpos);
					}
				}
				else
				{
					QPen   PenColorYellow(Qt::yellow);
					PenColorYellow.setStyle(Qt::DotLine);
					m_ListMeasureWindowItems[i].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect.toRect(), PenColorYellow);
					m_ListMeasureWindowItems[i].m_MeasureWindowItem->setTransformOriginPoint(m_ListMeasureWindowItems[i].m_MeasureWindowItem->rect().center());
					m_ListMeasureWindowItems[i].m_MeasureWindowItem->setRotation(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ModelAngle*(-1.0));
				}
			}
			else
			{
				double FormatTopLeftXpos, FormatTopLeftYpos;
				GetInspectionWindowFormatRectXY(Image, FormatTopLeftXpos, FormatTopLeftYpos);
				QRectF ROIRectA;
				QRectF ROIRect = Image.m_ListInspectionWindowResults.at(i).m_ROIRectRelatetToFormatRect;

				ROIRectA.setX(FormatTopLeftXpos + ROIRect.x());
				ROIRectA.setY(FormatTopLeftYpos + ROIRect.y());
				ROIRectA.setWidth(ROIRect.width());
				ROIRectA.setHeight(ROIRect.height());
				ZoomRect(ROIRectA, ZoomFactor);
				m_ListMeasureWindowItems[i].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(ROIRectA.toRect(), PenColorRed);
				TextXpos = static_cast<int>((FormatTopLeftXpos + Image.m_ListInspectionWindowResults[i].m_ROIRectRelatetToFormatRect.x()) * ZoomFactor + 0.5);
				TextMeasuringResults = tr("<>%1 Not Found</>").arg(Image.m_ListInspectionWindowResults.at(i).m_ModelName);
				m_ListMeasureWindowItems[i].m_MeasureWindowResultText = m_GrapicSenceLiveImage->addText("");
				SetHTMLGraphicTextItem(m_ListMeasureWindowItems[i].m_MeasureWindowResultText, TextMeasuringResults, TextXpos, PrintYpos,Qt::red);
				
			}
		}
		ClearGraphicItem(m_LineItemHoseMiddlePos);
		LineHoseMiddlePos.setLine(0, static_cast<int>(Image.m_HoseMiddlePosition * ZoomFactor + 0.5), rect().width() - 1, static_cast<int>(Image.m_HoseMiddlePosition * ZoomFactor + 0.5));
		m_LineItemHoseMiddlePos = m_GrapicSenceLiveImage->addLine(LineHoseMiddlePos, PenColorBlue);
    }
}



void LiveImageView::GetInspectionWindowFormatRectXY(const ImageMetaData &Image, double &XLeftTopPos, double &YLeftTopPos)
{
	for (int i = 0; i < Image.m_ListInspectionWindowResults.count(); i++)
	{
		if (Image.m_ListInspectionWindowResults[i].m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
		{
			XLeftTopPos = Image.m_ListInspectionWindowResults[i].m_Results.m_ResultXPos - Image.m_ListInspectionWindowResults[i].m_Results.m_ObjectSizeInX / 2.0;
			YLeftTopPos = Image.m_ListInspectionWindowResults[i].m_Results.m_ResultYPos - Image.m_ListInspectionWindowResults[i].m_Results.m_ObjectSizeInY / 2.0;
			break;
		}
	}
}


void LiveImageView::SetHTMLGraphicTextItem(QGraphicsTextItem *Item, QString &text, int xpos, int ypos, Qt::GlobalColor Color)
{
	Item->setHtml(QString("<p style='background:rgba(120,120,120, 100%);'>" + text +QString("</p>")));
	Item->setDefaultTextColor(Color);
	Item->setPos(xpos, ypos);
}


void LiveImageView::DrawAllReferenceRects()
{
	if (GetImageData()->GetNumberInspectionWindows() > 1)
	{
		for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
			DrawReferenceRect(i);
	}
}


void LiveImageView::DrawReferenceRect(int InsepectionWindowIndex)
{
	ProductData *pProductData = GetImageData()->GetCurrentProductData();
	double ZoomFactor = GetImageData()->GetDisplayZoomFactorFormatImage();
	SubFormatData *pSubFormatData = NULL;
	InspectionWindow *pInspectionWindow = NULL;
	
	
	if (pProductData)
	{
		pSubFormatData = pProductData->GetSubFormat(GetImageData()->GetCameraIndex());
		if (pSubFormatData)
		{
			pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(InsepectionWindowIndex);
			if (pInspectionWindow)
			{
				QRectF CurrentRect = pInspectionWindow->m_ReferenceRect;
				QRectF TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos;
				QPen   PenColorGreen(Qt::darkGreen);
				QPen   PenColorYellow(Qt::yellow);
				//QPen   PenColorBlue(Qt::blue);
				QBrush BrushColor(Qt::white);

				/*QRectF ROIRectA;
				QRectF ROIRect = pInspectionWindow->m_ROIRectRelatetToFormatRect;

				ROIRectA.setX(MasterRectX + ROIRect.x());
				ROIRectA.setY(MasterRectY + ROIRect.y());
				ROIRectA.setWidth(ROIRect.width());
				ROIRectA.setHeight(ROIRect.height());
				ROIRectA = ROIRectA;
				*/
				
				PenColorGreen.setWidth(2);
				PenColorYellow.setStyle(Qt::DotLine);
				GetRectSupportPoints(CurrentRect, TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos);
				if (ZoomFactor != 1.0)
				{
					ZoomRect(CurrentRect, ZoomFactor);
					ZoomRect(TopSquareMiddelPos, ZoomFactor);
					ZoomRect(BottomSquareMiddelPos, ZoomFactor);
					ZoomRect(LeftSquareMiddelPos, ZoomFactor);
					ZoomRect(RightSquareMiddelPos, ZoomFactor);
					//ZoomRect(ROIRect, ZoomFactor);
				}
				ClearGraphicItem(m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemRight);
				m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemRight = NULL;
				ClearGraphicItem(m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemLeft);
				m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemLeft = NULL;
				ClearGraphicItem(m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemBot);
				m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemBot = NULL;
				ClearGraphicItem(m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemTop);
				m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemTop = NULL;
				ClearGraphicItem(m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItem);
				m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItem = NULL;

				ClearGraphicItem(m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemROI);
			    m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemROI = NULL;
				
				if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
				{//master rect nur zeichenen wen mindesten zwei Messfenter gesetzt
					if(GetImageData()->GetNumberInspectionWindows() > 2)
					   m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColorYellow);
				}
				else
				{
					//m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemROI = m_GrapicSenceLiveImage->addRect(ROIRect, PenColorBlue);
					m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColorGreen);
					m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItem->setFlag(QGraphicsItem::ItemIsSelectable);
					m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemRight = m_GrapicSenceLiveImage->addRect(RightSquareMiddelPos, PenColorGreen, BrushColor);
					m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemLeft = m_GrapicSenceLiveImage->addRect(LeftSquareMiddelPos, PenColorGreen, BrushColor);
					m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemBot = m_GrapicSenceLiveImage->addRect(BottomSquareMiddelPos, PenColorGreen, BrushColor);
					m_ListMeasureWindowItems[InsepectionWindowIndex].m_MeasureWindowItemTop = m_GrapicSenceLiveImage->addRect(TopSquareMiddelPos, PenColorGreen, BrushColor);
				}
			}
		}
	}
}


void LiveImageView::DrawFormatRect()
{
	ProductData *pProductData = GetImageData()->GetCurrentProductData();
	double ZoomFactor = GetImageData()->GetDisplayZoomFactorFormatImage();
	int InspectionIDMasterRect = 0;
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetImageData()->GetCameraIndex());
		if (pSubFormatData)
		{
			InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByID(InspectionIDMasterRect);
			if (pInspectionWindow)
			{
				int index= pSubFormatData->GetIndexByID(InspectionIDMasterRect);
				if (index >= 0)
				{
					QRectF CurrentRect = pInspectionWindow->m_ReferenceRect;
					QPen   PenColorYellow(Qt::yellow);

					PenColorYellow.setStyle(Qt::DotLine);
					if (ZoomFactor != 1.0)
						ZoomRect(CurrentRect, ZoomFactor);
					ClearGraphicItem(m_ListMeasureWindowItems[index].m_MeasureWindowItem);
					m_ListMeasureWindowItems[index].m_MeasureWindowItem = NULL;
					m_ListMeasureWindowItems[index].m_MeasureWindowItem = m_GrapicSenceLiveImage->addRect(CurrentRect, PenColorYellow);
				}
			}
		}
	}
}


void LiveImageView::DeleteSelectedRect()
{
	QList<QGraphicsItem *> ListSelected = m_GrapicSenceLiveImage->selectedItems();
	if (ListSelected.count() > 0)
	{
		QGraphicsItem *pMeasureWindowItem = ListSelected.at(0);
		if (pMeasureWindowItem)
		{
			for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++) 
			{
				InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
				if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID> INSPECTION_ID_FORMAT_WINDOW)
				{
					if (m_ListMeasureWindowItems[i].m_MeasureWindowItem == (QGraphicsRectItem * )(pMeasureWindowItem))
					{
						if (ShowInspectionDialog(i, true) == QDialog::Accepted)
						{
							QString ReferenceLocation = GetImageData()->GetReferenceLocation() + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID);
							QDir DeleteDir(ReferenceLocation);
							GetImageData()->RemoveInspectionRect(pInspectionWindow->m_InspectionWindowID);
							DeleteDir.removeRecursively();
							SetMeasureWindowRectGraphicsItem();
							GetImageData()->CalculateFormatRect();//muss neu berechnet werden
							GetImageData()->WriteProductData();
							DrawAllReferenceRects();
						}
						break;
					}
				}
			}
		}
	}
}


void LiveImageView::YCenterAllMeasureWindow()
{
	QList<QGraphicsItem *> ListSelected = m_GrapicSenceLiveImage->selectedItems();
	if (ListSelected.count() > 0)
	{
		QGraphicsItem *pMeasureWindowItem = ListSelected.at(0);
		if (pMeasureWindowItem)
		{
			for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
			{
				InspectionWindow *pSelectedInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
				if (pSelectedInspectionWindow && pSelectedInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
				{
					if (m_ListMeasureWindowItems[i].m_MeasureWindowItem == (QGraphicsRectItem *)(pMeasureWindowItem))
					{
						double Ypos = pSelectedInspectionWindow->m_ReferenceRect.center().y();
						for (int k = 0; k < GetImageData()->GetNumberInspectionWindows(); k++)
						{
							InspectionWindow *pInspectWindow=GetImageData()->GetInspectionWindowByIndex(k);
							if (pInspectWindow)
							{
								QPointF CenterPoint = pInspectWindow->m_ReferenceRect.center();

								CenterPoint.setY(Ypos);
								pInspectWindow->m_ReferenceRect.moveCenter(CenterPoint);
							}
						}
						break;
					}
				}
			}
			DrawAllReferenceRects();
		}
	}
}


void LiveImageView::ZoomRect(QRectF &rect, double ZoomFactor)
{
	QPointF TopLeft, BottomRight;
	TopLeft     = rect.topLeft()     * ZoomFactor;
	BottomRight = rect.bottomRight() * ZoomFactor;
	rect.setTopLeft(TopLeft);
	rect.setBottomRight(BottomRight);

}


void LiveImageView::GetRectSupportPoints(QRectF &CurrentRect, QRectF &TopSquareMiddelPos, QRectF &BottomSquareMiddelPos, QRectF &LeftSquareMiddelPos, QRectF &RightSquareMiddelPos)
{
	if (GetImageData())
	{
		double Zoom = GetImageData()->GetDisplayZoomFactorFormatImage(); 
		int AOIRectWidth2;
		int AOIRectHeight2;
		int    ScanWidth2;
		int    ScanWidth;


		AOIRectWidth2 = static_cast<int>(CurrentRect.width() / 2.0 + 0.5);
		AOIRectHeight2 = static_cast<int>(CurrentRect.height() / 2.0 + 0.5);
		ScanWidth2 = ((SUPPORT_RECT_SIZE_IN_PIXEL / 2.0 + 0.5) / Zoom);//
		ScanWidth = ScanWidth2 * 2;

		TopSquareMiddelPos.setX(CurrentRect.topLeft().x() + AOIRectWidth2 - ScanWidth2); //top left position
		TopSquareMiddelPos.setY(CurrentRect.topLeft().y() - ScanWidth2);                           //top left position
		TopSquareMiddelPos.setWidth(ScanWidth);
		TopSquareMiddelPos.setHeight(ScanWidth);

		BottomSquareMiddelPos.setX(CurrentRect.bottomLeft().x() + AOIRectWidth2 - ScanWidth2);
		BottomSquareMiddelPos.setY(CurrentRect.bottomLeft().y() - ScanWidth2);
		BottomSquareMiddelPos.setWidth(ScanWidth);
		BottomSquareMiddelPos.setHeight(ScanWidth);

		LeftSquareMiddelPos.setX(CurrentRect.topLeft().x() - ScanWidth2);
		LeftSquareMiddelPos.setY(CurrentRect.topLeft().y() + AOIRectHeight2 - ScanWidth2);
		LeftSquareMiddelPos.setWidth(ScanWidth);
		LeftSquareMiddelPos.setHeight(ScanWidth);

		RightSquareMiddelPos.setX(CurrentRect.bottomRight().x() - ScanWidth2);
		RightSquareMiddelPos.setY(CurrentRect.bottomRight().y() - AOIRectHeight2 - ScanWidth2);
		RightSquareMiddelPos.setWidth(ScanWidth);
		RightSquareMiddelPos.setHeight(ScanWidth);
	}
}


void LiveImageView::GetLineSupportPoint(QLine &lineX, QLine &lineY,QRectF &LineRectPoint)
{
	double rectWidth = 20;
	LineRectPoint.setX(lineX.x1() - rectWidth / 2);
	LineRectPoint.setY(lineY.y1() - rectWidth / 2);
	LineRectPoint.setWidth(rectWidth);
	LineRectPoint.setHeight(rectWidth);
}


void LiveImageView::ClearGraphicItem(QGraphicsItem *Item)
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


void LiveImageView::mousePressEvent(QMouseEvent * e)
{
	if (!m_IsFormatLiveImage)
	{
		if (e->button() == Qt::LeftButton)
		{
			m_ClickPos = mapToScene(e->pos());
			if (!m_MeasureRulerIsActive)
			  SetCurserForMeasureWindowROI(m_ClickPos.x(), m_ClickPos.y());
			else
              SetCurserForMeasureRuler(m_ClickPos.x(), m_ClickPos.y());
			m_MousePressEventIsClicked = true;
		}
		else
		{
			if (e->button() == Qt::RightButton)
			{
				m_ClickPos = mapToScene(e->pos());
				SetCurserForMeasureWindowROI(m_ClickPos.x(), m_ClickPos.y());
				ShowInspectionDialog(m_CurrentInspectionWindowIndex);
			}
		}
	}
	QGraphicsView::mousePressEvent(e);
}


void LiveImageView::mouseMoveEvent(QMouseEvent * e)
{
	if (!m_IsFormatLiveImage)
	{
		QPointF CurrentPoint = mapToScene(e->pos());

		if (GetImageData())
		{
			if (CurrentPoint.x() >= 0 && CurrentPoint.y() >= 0 && CurrentPoint.x() < m_Pixmap->pixmap().width() && CurrentPoint.y() < m_Pixmap->pixmap().height())
			{
				if (m_MousePressEventIsClicked)
				{
					if (!m_MeasureRulerIsActive)
					{
						double Zoom = GetImageData()->GetDisplayZoomFactorFormatImage();
						int NewXPos = (static_cast<int>(CurrentPoint.x() / Zoom + 0.5));
						int NewYPos = (static_cast<int>(CurrentPoint.y() / Zoom + 0.5));
						if (NewXPos < 0)NewXPos = 0;
						if (NewYPos < 0)NewYPos = 0;

						if (m_CurserPosition == CURSER_POSITION_MOVE_FULL_FORMAT)
						{//das komplette formatfenster wird bewegt
							for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
							{
							    MoveAndResizeMeasureWindow(NewXPos, NewYPos, CURSER_POSITION_MOVE_IN_ROI, i, Zoom);
								DrawReferenceRect(i);
							}
						}
						else
						{//ein einzlner Block wird bewegt
							MoveAndResizeMeasureWindow(NewXPos, NewYPos, m_CurserPosition, m_CurrentInspectionWindowIndex, Zoom);
							DrawReferenceRect(m_CurrentInspectionWindowIndex);
						}
						m_ClickPos.setX(static_cast<int>(NewXPos*Zoom + 0.5));
		                m_ClickPos.setY(static_cast<int>(NewYPos*Zoom + 0.5));
					}
					else
					{//Messlinear ist aktiv
						if (m_CurserPositionRuler == CURSER_POSITION_LEFT_LINE)
						{
							m_LeftMeasureLine.setLine(CurrentPoint.x(), 0, CurrentPoint.x(), m_CurrentImageHeight);// m_CurrentImageHeight);
							m_TopMeasureLine.setLine(0, CurrentPoint.y(), m_CurrentImageWidth, CurrentPoint.y());
							DrawMeasureLines();
						}
						else
						{
							if (m_CurserPositionRuler == CURSER_POSITION_RIGHT_LINE)
							{
								m_RightMeasureLine.setLine(CurrentPoint.x(), 0, CurrentPoint.x(), m_CurrentImageHeight);
								m_BotMeasureLine.setLine(0, CurrentPoint.y(), m_CurrentImageWidth, CurrentPoint.y());
								DrawMeasureLines();
							}
						}
					}
				}
			}
		}
	}
	QGraphicsView::mouseMoveEvent(e);
}


void LiveImageView::mouseReleaseEvent(QMouseEvent * e)
{
	if (!m_IsFormatLiveImage)
	{
		if (m_MousePressEventIsClicked)
		{
			if (m_CurserPosition != CURSER_POSITION_MOVE_FULL_FORMAT)
			{
				GetImageData()->CalculateFormatRect();
				DrawFormatRect();
			}
			m_MousePressEventIsClicked = false;
			QApplication::restoreOverrideCursor();
			m_CurserPosition = CURSER_POSITION_NOT_ON_ROI;
    	}
	}
	QGraphicsView::mouseReleaseEvent(e);
}


void LiveImageView::SetMeasureRulerIsActive(bool Active)
{
	if (!m_IsFormatLiveImage)
	{//Referenzansicht
		m_MeasureRulerIsActive = Active;
		if (m_MeasureRulerIsActive)
		{
			m_LeftMeasureLine.setLine(m_CurrentImageWidth/2.0, 0, m_CurrentImageWidth/2.0, m_CurrentImageHeight);
			m_RightMeasureLine.setLine(m_CurrentImageWidth / 2.0 + 200, 0, m_CurrentImageWidth / 2.0 + 200, m_CurrentImageHeight);

			m_TopMeasureLine.setLine(0, m_YposSupportPointMeasureRuler, m_CurrentImageWidth, m_YposSupportPointMeasureRuler);
			m_BotMeasureLine.setLine(0, m_YposSupportPointMeasureRuler, m_CurrentImageWidth, m_YposSupportPointMeasureRuler);
			ClearAllMeasureWindowGraphicsItems();
			DrawMeasureLines();
		}
		else
		{
			ClearAllMeasureRulerItems();
			SetMeasureWindowRectGraphicsItem();
			DrawAllReferenceRects();
		}
	}
}


void LiveImageView::ClearAllMeasureRulerItems()
{
	ClearGraphicItem(m_LineItemLeftMeasureLine);
	m_LineItemLeftMeasureLine = NULL;
	ClearGraphicItem(m_LineItemRightMeasureLine);
	m_LineItemRightMeasureLine = NULL;
	ClearGraphicItem(m_RectItemLeftMeasureLine);
	m_RectItemLeftMeasureLine = NULL;
	ClearGraphicItem(m_RectItemRightMeasureLine);
	m_RectItemRightMeasureLine = NULL;
	ClearGraphicItem(m_TextItemHorMeasureDistance);
	m_TextItemHorMeasureDistance = NULL;
	ClearGraphicItem(m_LineItemBotMeasureLine);
	m_LineItemBotMeasureLine = NULL;
	ClearGraphicItem(m_LineItemTopMeasureLine);
	m_LineItemTopMeasureLine = NULL;
}


void LiveImageView::DrawMeasureLines()
{
	QPen   PenColorBlue(Qt::blue);
	QBrush BrushColor(Qt::gray);
	QRectF LineLeftRect, LineRightRect;
	double Zoom = GetImageData()->GetDisplayZoomFactorFormatImage();
	QString Text;
	QFont Font("Times", 20, QFont::Bold);
	double DistanceHorInMM = (abs(m_RightMeasureLine.x1() - m_LeftMeasureLine.x1()) / Zoom) * GetImageData()->GetPixelSize();
	double DistanceVerInMM = (abs(m_TopMeasureLine.y1() - m_BotMeasureLine.y1()) / Zoom) * GetImageData()->GetPixelSize();
	QFontMetrics fm(Font);
	
	ClearAllMeasureRulerItems();
	Font.setBold(true);
	Text = QString("B=%1mm  H=%2mm").arg(DistanceHorInMM, 0, 'f', 2).arg(DistanceVerInMM, 0, 'f', 2);
	
	m_TextItemHorMeasureDistance = m_GrapicSenceLiveImage->addText("");
	m_TextItemHorMeasureDistance->setHtml(QString("<p style='background:rgba(128,128,128, 100%);'>" + Text + QString("</p>")));
	m_TextItemHorMeasureDistance->setDefaultTextColor(Qt::blue);
	m_TextItemHorMeasureDistance->setFont(Font);
	m_TextItemHorMeasureDistance->setPos((m_RightMeasureLine.x1() + m_LeftMeasureLine.x1())/2.0 - (fm.horizontalAdvance(Text) /2.0), m_TopMeasureLine.y1() - fm.height()-20);

	GetLineSupportPoint(m_LeftMeasureLine, m_TopMeasureLine,LineLeftRect);
	GetLineSupportPoint(m_RightMeasureLine, m_BotMeasureLine, LineRightRect);

	m_LineItemLeftMeasureLine  = m_GrapicSenceLiveImage->addLine(m_LeftMeasureLine, PenColorBlue);
	m_LineItemRightMeasureLine = m_GrapicSenceLiveImage->addLine(m_RightMeasureLine, PenColorBlue);

	m_LineItemTopMeasureLine = m_GrapicSenceLiveImage->addLine(m_TopMeasureLine, PenColorBlue);
	m_LineItemBotMeasureLine = m_GrapicSenceLiveImage->addLine(m_BotMeasureLine, PenColorBlue);

	m_RectItemLeftMeasureLine  = m_GrapicSenceLiveImage->addEllipse(LineLeftRect,  PenColorBlue, BrushColor);
	m_RectItemRightMeasureLine = m_GrapicSenceLiveImage->addEllipse(LineRightRect, PenColorBlue, BrushColor);
}


void LiveImageView::SetCurserForMeasureRuler(int XPos, int YPos)
{
	QRectF LineLeftRect, LineRightRect;

	GetLineSupportPoint(m_LeftMeasureLine, m_TopMeasureLine, LineLeftRect);
	GetLineSupportPoint(m_RightMeasureLine, m_BotMeasureLine, LineRightRect);
	if (LineLeftRect.contains(XPos, YPos))
	{
		QApplication::setOverrideCursor(Qt::SizeAllCursor);
		m_CurserPositionRuler = CURSER_POSITION_LEFT_LINE;
		
	}
	else
	{
		if (LineRightRect.contains(XPos, YPos))
		{
			QApplication::setOverrideCursor(Qt::SizeAllCursor);
			m_CurserPositionRuler = CURSER_POSITION_RIGHT_LINE;
		}
		else
		{
			QApplication::restoreOverrideCursor();
			m_CurserPositionRuler = CURSER_POSITION_NOT_ON_ROI;
		}
	}
}


void LiveImageView::MoveAndResizeMeasureWindow(int NewXPos, int NewYPos,  int CurserPosition, int InspectionWindowIndex, double Zoom)
{
	QRectF ROIRect = GetImageData()->GetInspectionRectByIndex(InspectionWindowIndex);
	int x = ROIRect.x();
	int y = ROIRect.y();
	int w = ROIRect.width();
	int h = ROIRect.height();
	int DeltaX, NewWidth;
	int DeltaY, NewHeight;
	int ImageHeight = static_cast<int>(m_CurrentImageHeight / Zoom);
	int ImageWidth  = static_cast<int>(m_CurrentImageWidth / Zoom);

	switch (CurserPosition)
	{
	case CURSER_POSITION_RESIZE_TOP:
		DeltaY = ROIRect.y() - NewYPos;
		NewHeight = ROIRect.height() + DeltaY;
		if (NewYPos > 0 && NewHeight > MINIMUM_ROI_SIZE_IN_PIXEL)
		{
			ROIRect.setX(x);
			ROIRect.setY(NewYPos);
			ROIRect.setHeight(NewHeight);
			ROIRect.setWidth(w);
			GetImageData()->SetInspectionRectByIndex(ROIRect, InspectionWindowIndex);
		}
		break;
	case CURSER_POSITION_RESIZE_BOTTOM:
		DeltaY = ROIRect.y() + ROIRect.height() - NewYPos;
		NewHeight = ROIRect.height() - DeltaY;
		if (NewYPos < ImageHeight && NewHeight > MINIMUM_ROI_SIZE_IN_PIXEL)
		{
			ROIRect.setX(x);
			ROIRect.setY(y);
			ROIRect.setHeight(NewHeight);
			ROIRect.setWidth(w);
			GetImageData()->SetInspectionRectByIndex(ROIRect, InspectionWindowIndex);
		}
		break;
	case CURSER_POSITION_RESIZE_LEFT:
		DeltaX = ROIRect.x() - NewXPos;
		NewWidth = ROIRect.width() + DeltaX;
		if (NewXPos > 0 && NewWidth > MINIMUM_ROI_SIZE_IN_PIXEL)
		{
			ROIRect.setX(NewXPos);
			ROIRect.setY(y);
			ROIRect.setHeight(h);
			ROIRect.setWidth(NewWidth);
			GetImageData()->SetInspectionRectByIndex(ROIRect, InspectionWindowIndex);
		}
		break;
	case CURSER_POSITION_RESIZE_RIGHT:
		DeltaX = NewXPos - (ROIRect.x() + ROIRect.width());
		NewWidth = ROIRect.width() + DeltaX;
		if (NewXPos < ImageWidth && NewWidth > MINIMUM_ROI_SIZE_IN_PIXEL)
		{
			ROIRect.setX(x);
			ROIRect.setY(y);
			ROIRect.setHeight(h);
			ROIRect.setWidth(NewWidth);
			GetImageData()->SetInspectionRectByIndex(ROIRect, InspectionWindowIndex);
		}
		break;
	case CURSER_POSITION_MOVE_IN_ROI:
		MoveROIRect(ROIRect, NewXPos, NewYPos, InspectionWindowIndex, Zoom);
		break;
	default:
		break;
	}
}


void LiveImageView::SetCurserForMeasureWindowROI(int x, int y)
{
	if (GetImageData())
	{
		QRectF AOIRect;
		QRectF AOIFormatRect;
		QRectF TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos;
		double Zoom = GetImageData()->GetDisplayZoomFactorFormatImage();
		int    XPos = static_cast<int>(x / Zoom + 0.5);
		int    YPos = static_cast<int>(y / Zoom + 0.5);
		bool IsIn = false;
		InspectionWindow *pInspectionWindow = NULL;
		

		for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
		{
			pInspectionWindow=GetImageData()->GetInspectionWindowByIndex(i);
			if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
			{
				AOIRect = GetImageData()->GetInspectionRectByIndex(i);
				m_CurrentInspectionWindowIndex = i;
				GetRectSupportPoints(AOIRect, TopSquareMiddelPos, BottomSquareMiddelPos, LeftSquareMiddelPos, RightSquareMiddelPos);

				if (TopSquareMiddelPos.contains(XPos, YPos))
				{
					QApplication::setOverrideCursor(Qt::SizeVerCursor);
					m_CurserPosition = CURSER_POSITION_RESIZE_TOP;
					IsIn = true;
					break;
				}
				else
					if (BottomSquareMiddelPos.contains(XPos, YPos))
					{
						QApplication::setOverrideCursor(Qt::SizeVerCursor);
						m_CurserPosition = CURSER_POSITION_RESIZE_BOTTOM;
						IsIn = true;
						break;
					}
					else
						if (LeftSquareMiddelPos.contains(XPos, YPos))
						{
							QApplication::setOverrideCursor(Qt::SizeHorCursor);
							m_CurserPosition = CURSER_POSITION_RESIZE_LEFT;
							IsIn = true;
							break;
						}
						else
							if (RightSquareMiddelPos.contains(XPos, YPos))
							{
								QApplication::setOverrideCursor(Qt::SizeHorCursor);
								m_CurserPosition = CURSER_POSITION_RESIZE_RIGHT;
								IsIn = true;
								break;
							}
							else
								if (AOIRect.contains(XPos, YPos))
								{
									QApplication::setOverrideCursor(Qt::SizeAllCursor);
									m_CurserPosition = CURSER_POSITION_MOVE_IN_ROI;
									IsIn = true;
									break;
								}
								else
								{
									QApplication::restoreOverrideCursor();
									m_CurserPosition = CURSER_POSITION_NOT_ON_ROI;
								}
			}
		}
		if (!IsIn)
		{
			m_CurrentInspectionWindowIndex = -1;
			InspectionWindow *pFormatRect=GetImageData()->GetInspectionWindowByID(0);
			if (pFormatRect)
			{
				AOIRect = pFormatRect->m_ReferenceRect;
				if (AOIRect.contains(XPos, YPos))
				{
					QApplication::setOverrideCursor(Qt::PointingHandCursor);
					m_CurserPosition = CURSER_POSITION_MOVE_FULL_FORMAT;
				}
				else
				{
					QApplication::restoreOverrideCursor();
					m_CurserPosition = CURSER_POSITION_NOT_ON_ROI;
				}
			}
		}
	}
}


int LiveImageView::ShowInspectionDialog(int CurrentInspectionWindowIndex,bool DeleteButton)
{
	if (CurrentInspectionWindowIndex >= 0)
	{
		QApplication::restoreOverrideCursor();
		QString CancelText  = tr("Cancel");
		QString ApplyText   = tr("Apply");

		if(DeleteButton)
			ApplyText = tr("Delete");
		if (m_PopupDialogInspectionRectDialog == NULL)
			m_PopupDialogInspectionRectDialog = new PopupDialogInspectionRectDialog(GetImageData(),(QWidget*)(GetImageData()->GetMainAppPrintCheck()->GetMainGUIPrintCheck()));
		m_PopupDialogInspectionRectDialog->SetInspectionRectID(CurrentInspectionWindowIndex, DeleteButton, CancelText, ApplyText);
		return m_PopupDialogInspectionRectDialog->exec();
	}
	else
		return QDialog::Rejected;
}


void LiveImageView::MoveROIRect(QRectF &ROIRect, int NewXPos, int NewYPos, int InspectionWindowIndex, double Zoom)
{
	if (GetImageData())
	{
		QRectF NewROIRect;
		int ImageHeight = static_cast<int>(m_CurrentImageHeight / Zoom);
		int ImageWidth  = static_cast<int>(m_CurrentImageWidth / Zoom);
		int DeltaX      = NewXPos - static_cast<int>(m_ClickPos.x() / Zoom + 0.5);
		int DeltaY      = NewYPos - static_cast<int>(m_ClickPos.y() / Zoom + 0.5);
		
		NewXPos = ROIRect.x() + DeltaX;
		NewYPos = ROIRect.y() + DeltaY;
		//grenzen überprüfen
		if (NewXPos < 0)NewXPos = 0;
		if (NewYPos < 0)NewYPos = 0;
		if ((NewXPos + ROIRect.width()) >= ImageWidth)
			NewXPos = ImageWidth - ROIRect.width();
		if ((NewYPos + ROIRect.height()) >= ImageHeight)
			NewYPos = ImageHeight - ROIRect.height();
		NewROIRect.setX(NewXPos);
		NewROIRect.setY(NewYPos);
		NewROIRect.setWidth(ROIRect.width());
		NewROIRect.setHeight(ROIRect.height());
		GetImageData()->SetInspectionRectByIndex(NewROIRect, InspectionWindowIndex);
	}
}







