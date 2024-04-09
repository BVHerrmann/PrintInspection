#include "ErrorImageView.h"
#include "ImageData.h"
#include "GlobalConst.h"
#include "MainAppPrintCheck.h"



ErrorImageView::ErrorImageView(ImageData *pImageData) : QGraphicsView()
, m_GrapicSenceLiveImage(NULL)
, m_Pixmap(NULL)
, m_ImageData(NULL)
, m_MeasureWindowItemFormatRect(NULL)
, m_ErrorTextHeadLine(NULL)
{
	m_GrapicSenceLiveImage = new QGraphicsScene(this);
	m_Pixmap               = m_GrapicSenceLiveImage->addPixmap(QPixmap());
	m_ImageData            = pImageData;
	setScene(m_GrapicSenceLiveImage);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setMouseTracking(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}


ErrorImageView::~ErrorImageView()
{
}


void ErrorImageView::SetMeasurWindowRectErrorTextItems()
{
	int NumRects = GetImageData()->GetNumberInspectionWindows();

	ClearAllMeasurWindowRectErrorTextItems();
	if (NumRects != m_ListMeasurWindowRectErrorText.count())
	{
		m_ListMeasurWindowRectErrorText.clear();
		for (int i = 0; i < NumRects; i++)
		{
			MeasurWindowRectErrorText NewItem;
			m_ListMeasurWindowRectErrorText.append(NewItem);
		}
	}
}

//Lösch alle Graphicobjekte aus dem Overlay des Bildes
void ErrorImageView::ClearAllMeasurWindowRectErrorTextItems()
{
	for (int i = 0; i < m_ListMeasurWindowRectErrorText.count(); i++)
	{
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextModelNotFound);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextModelNotFound = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextPositionNotOk);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextPositionNotOk = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextPrintNotOk);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextPrintNotOk = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatLenghtNotOk);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatLenghtNotOk = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockLenghtNotOk);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockLenghtNotOk = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockHeightNotOk);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockHeightNotOk = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatNotFound);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatNotFound = NULL;
		ClearGraphicItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextLineMissing);
		m_ListMeasurWindowRectErrorText[i].m_ErrorTextLineMissing = NULL;
		ClearGraphicItem(m_ErrorTextHeadLine);
		m_ErrorTextHeadLine = NULL;
    }
	ClearGraphicItem(m_MeasureWindowItemFormatRect);
	m_MeasureWindowItemFormatRect = NULL;
}

//Löscht das Fehlerbild aus der Anzeige
void ErrorImageView::ClearImage()
{
	if (m_Pixmap)
		m_Pixmap->setPixmap(QPixmap());
	ClearAllMeasurWindowRectErrorTextItems();
}


void ErrorImageView::SlotShowErrorImage(const ImageMetaData &Image)
{
	if (m_Pixmap)
	{
		m_Pixmap->setPixmap(QPixmap::fromImage(Image.m_Image));
		QRectF BRect   = m_Pixmap->boundingRect();
		double NewYPos = (rect().height() - BRect.height()) / 2.0;
		if (NewYPos > 0.0)//Bild mittig Horizontal positionieren
		{
			BRect.setY(NewYPos*-1.0);
			setSceneRect(BRect);
		}
		else
			setSceneRect(BRect);
		SetMeasurWindowRectErrorTextItems();
		DrawErrorInfo(Image);//Anzeige des Fehlertextes im Bild
		if (GetImageData()->GetSaveErrorImagePoolCondition() == SAVE_FORMAT_IMAGE_ONLY_BAD_IMAGES)
		{//Bild soll gespeichert werden
			ImageMetaData ResultsAndImage;

			ResultsAndImage.m_Pixmap = grab();//Kopiere Fensterinhalt
			for (int i = 0; i < Image.m_ListInspectionWindowResults.count(); i++)
			{
				InspectionWindow InspectionResults = Image.m_ListInspectionWindowResults.at(i);
				ResultsAndImage.m_ListInspectionWindowResults.append(InspectionResults);
			}
			GetImageData()->AppendInspectionResultsForStorage(ResultsAndImage);//zur List hinzufügen, die Liste wird dann vom Thread abgearbeitet der dann die Messdaten und das Bild speichert
		}
		GetImageData()->SlotStartToggelTimerShowLiveView();//starte Umschaltung auf das Fehlerbild
		GetImageData()->GetMainAppPrintCheck()->IncrementNumberErrorHose();//Fehlerzähler für den aktuellen Schlauch um eins erhöhen
		//abfrage kommt das Fehlerbild von der MeasureTaskFormatCheck, ist nur dann von Bedeutung wenn, die Anzeige für die Darstellung der Messergebnisse geöffnet wird
		if (Image.m_ListInspectionWindowResults.count() == 1)
		{//nur ein Messfenster vorhanden -> dann ist dies das Formatbild
			InspectionWindow InspectionResults = Image.m_ListInspectionWindowResults.at(0);
			if (InspectionResults.m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
			{//hier ist ein Fehler bei der Formaterkennung aufgetreten
				ImageMetaData ResultsAndImage;
				ResultsAndImage.m_ListInspectionWindowResults.append(InspectionResults);
				GetImageData()->AppendInspectionResultsForResultView(ResultsAndImage);//info für die Anzeig der Messergebnisse, aber nur wenn Dialog geöffnet/sichtbar
			}
		}
	}
}

//Anzeige der Fehlertexte 
void ErrorImageView::DrawErrorInfo(const ImageMetaData &Image)
{
	if (GetImageData())
	{
		double DisplayZoomFactor = GetImageData()->GetDisplayZoomFactorFormatImage();
		int TextYpos, TextXpos;
		QString TextMeasuringResults;
		
		ClearAllMeasurWindowRectErrorTextItems();
		TextMeasuringResults = tr("<>Print Error: %1  Image: %2  Hose: %3</>").arg(QDateTime::currentDateTime().toString(DATE_TIME_FORMAT)).arg(GetImageData()->GetFormatCounter()).arg(GetImageData()->GetNumberHose());
		m_ErrorTextHeadLine = m_GrapicSenceLiveImage->addText("");
		SetHTMLGraphicTextItem(m_ErrorTextHeadLine, TextMeasuringResults, 0, -50, Qt::red);
		if (Image.m_ListInspectionWindowResults.count() > 0)
		{
			for (int i = 0; i < Image.m_ListInspectionWindowResults.count(); i++)
			{
					if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_ModelFound))
					{
						double FormatTopLeftXpos, FormatTopLeftYpos;
						GetInspectionWindowFormatRectXY(Image, FormatTopLeftXpos, FormatTopLeftYpos);
						TextXpos = static_cast<int>((FormatTopLeftXpos + Image.m_ListInspectionWindowResults[i].m_ROIRectRelatetToFormatRect.x()) * DisplayZoomFactor + 0.5);
						TextYpos = static_cast<int>((FormatTopLeftYpos + Image.m_ListInspectionWindowResults[i].m_ROIRectRelatetToFormatRect.y()) * DisplayZoomFactor + 0.5);
						TextYpos = TextYpos + (Image.m_ListInspectionWindowResults[i].m_ReferenceRect.height()* DisplayZoomFactor + 0.5);
						TextMeasuringResults = tr("<>%1 Not Found</>").arg(Image.m_ListInspectionWindowResults[i].m_ModelName);
						m_ListMeasurWindowRectErrorText[i].m_ErrorTextModelNotFound =m_GrapicSenceLiveImage->addText("");
     					SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextModelNotFound, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
						
					}
					else
					{//Model found
						if (Image.m_ListInspectionWindowResults[i].m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
						{//Format rect
							QRectF CurrentRect;
							QPen   PenColorYellow(Qt::yellow);
							PenColorYellow.setStyle(Qt::DotLine);

							CurrentRect.setX(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ResultXPos - Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInX / 2.0);
							CurrentRect.setY(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ResultYPos - Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInY / 2.0);
							CurrentRect.setWidth(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInX);
							CurrentRect.setHeight(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ObjectSizeInY);
							if (DisplayZoomFactor != 1.0)
								ZoomRect(CurrentRect, DisplayZoomFactor);
							m_MeasureWindowItemFormatRect = m_GrapicSenceLiveImage->addRect(CurrentRect.toRect(), PenColorYellow);
							m_MeasureWindowItemFormatRect->setTransformOriginPoint(m_MeasureWindowItemFormatRect->rect().center());
							m_MeasureWindowItemFormatRect->setRotation(Image.m_ListInspectionWindowResults.at(i).m_Results.m_ModelAngle*(-1.0));
						}

						TextXpos = static_cast<int>((Image.m_ListInspectionWindowResults[i].m_Results.m_ResultXPos - Image.m_ListInspectionWindowResults[i].m_Results.m_ObjectSizeInX / 2.0) * DisplayZoomFactor + 0.5);
						TextYpos = static_cast<int>((Image.m_ListInspectionWindowResults[i].m_Results.m_ResultYPos - Image.m_ListInspectionWindowResults[i].m_Results.m_ObjectSizeInY / 2.0) * DisplayZoomFactor + 0.5);
						TextYpos = TextYpos + (Image.m_ListInspectionWindowResults[i].m_ReferenceRect.height()* DisplayZoomFactor + 0.5)+10;
						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_PrintOk))
						{
							TextMeasuringResults = tr("<>Print Error:%1</>").arg(Image.m_ListInspectionWindowResults[i].m_ModelName);
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextPrintNotOk = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextPrintNotOk, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
						}
						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_PositionOk))
						{
							TextMeasuringResults = tr("<>Offset Error:%1</>").arg(Image.m_ListInspectionWindowResults[i].m_ModelName);
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextPositionNotOk = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextPositionNotOk, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
						}
						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_FormatLenghtOk))
						{
							TextMeasuringResults = tr("<>%1 Lenght Error(%2mm)</>").arg(Image.m_ListInspectionWindowResults[i].m_ModelName).arg(Image.m_ListInspectionWindowResults[i].m_Results.m_LenghtErrorInMM,0,'f',2);
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatLenghtNotOk = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatLenghtNotOk, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
						}
						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_BlockWidthOk))
						{
							TextMeasuringResults = tr("<>%1 Width Error(%2mm)</>").arg(Image.m_ListInspectionWindowResults[i].m_ModelName).arg(Image.m_ListInspectionWindowResults[i].m_Results.m_LenghtErrorInMM, 0, 'f', 2);
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockLenghtNotOk = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockLenghtNotOk, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
							TextYpos = TextYpos + 50;
						}
						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_BlockHeightOk))
						{
							TextMeasuringResults = tr("<>%1 Height Error(%2mm)</>").arg(Image.m_ListInspectionWindowResults[i].m_ModelName).arg(Image.m_ListInspectionWindowResults[i].m_Results.m_HeightErrorInMM, 0, 'f', 2);
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockHeightNotOk = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextBlockHeightNotOk, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
						}
						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_FormatFound))
						{
							TextMeasuringResults = tr("<>Format not found</>");
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatNotFound = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextFormatNotFound, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
						}

						if (!(Image.m_ListInspectionWindowResults[i].m_Results.m_LineCheckOk))
						{
							TextMeasuringResults = tr("<>Printline Missing fehlt</>");
							m_ListMeasurWindowRectErrorText[i].m_ErrorTextLineMissing = m_GrapicSenceLiveImage->addText("");
							SetHTMLGraphicTextItem(m_ListMeasurWindowRectErrorText[i].m_ErrorTextLineMissing, TextMeasuringResults, TextXpos, TextYpos, Qt::red);
					    }
					}
			}
		}
	}
}


void ErrorImageView::GetInspectionWindowFormatRectXY(const ImageMetaData &Image, double &XLeftTopPos, double &YLeftTopPos)
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


void ErrorImageView::ZoomRect(QRectF &rect, double ZoomFactor)
{
	QPointF TopLeft, BottomRight;
	TopLeft = rect.topLeft()     * ZoomFactor;
	BottomRight = rect.bottomRight() * ZoomFactor;
	rect.setTopLeft(TopLeft);
	rect.setBottomRight(BottomRight);
}


void ErrorImageView::SetHTMLGraphicTextItem(QGraphicsTextItem *Item, QString &text, int xpos, int ypos, Qt::GlobalColor Color)
{
	QFont Font("Times", 12, QFont::Bold);
	Font.setBold(true);
	Item->setHtml(QString("<p style='background:rgba(255,255,255, 100%);'>" + text + QString("</p>")));
	Item->setDefaultTextColor(Color);
	Item->setFont(Font);
	Item->setPos(xpos, ypos);
}


void ErrorImageView::ClearGraphicItem()
{
	QList<QGraphicsItem *> items = m_GrapicSenceLiveImage->items();
	for (int i = 0; i < items.size(); i++)
	{
		QGraphicsItem *item = items.at(i);
		if (item != m_Pixmap)
		{
			m_GrapicSenceLiveImage->removeItem(item);
			delete item;
			break;
		}
	}
}

void ErrorImageView::ClearGraphicItem(QGraphicsItem *Item)
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

