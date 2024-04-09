#include "MeasureTaskPrintCheck.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "SharedMemoryVideoData.h"
#include "VideoHeader.h"
#include "InspectionWindow.h"
#include "MatchingShapeBased.h"
#include "MeasurementResult.h"
#include "SubFormatData.h"
#include "MainAppPrintCheck.h"
#include "MainLogic.h"
#include "PrintLineDetector.h"



MeasureTaskPrintCheck::MeasureTaskPrintCheck(ImageData *pImageData) : QThread()
,m_ImageData(NULL)
,m_TerminateInspection(false)
,m_SharedMemoryFormatImage(NULL)
,m_EventTerminateThread(NULL)
,m_EnableMeasuring(true)
,m_TimeStampFormatImageInMuSec(0)
,m_FormatImageWidth(0)
,m_FormatImageHeight(0)
,m_HoseMiddlePosition(0.0)
{
	m_EventTerminateThread         = CreateEventW(NULL, FALSE, FALSE, NULL);//thread aus dem wartezustand bringen und dann terminieren, wenn Programm beendet wird
	m_ImageData                    = pImageData;
	m_SharedMemoryFormatImage      = new SharedMemoryVideoData();
	m_SharedMemoryFormatImage->SetKeyName(QString("%1%2").arg(SHARED_MEMORY_KEYNAME_FORMAT_IMAGE).arg(GetImageData()->GetCameraIndex() + 1));//Shared Memory für das Formatbild
		
	m_Colors.resize(256);
	for (int i = 0; i < m_Colors.size(); i++)
		m_Colors[i] = 0xff000000 | i << 16 | i << 8 | i;
}


MeasureTaskPrintCheck::~MeasureTaskPrintCheck()
{
	WaitForFinshed();
}

//Beendet die Task in dem die Druckprüfung durchgeführt wird
void MeasureTaskPrintCheck::WaitForFinshed()
{
	if (isRunning())
	{//thread läuft noch
		if (m_EventTerminateThread)
			SetEvent(m_EventTerminateThread);
		m_TerminateInspection = true;
		m_WaitLiveImageViewIsDisable.lock();
		m_WaitConditionLiveViewIsDisable.wait(&m_WaitLiveImageViewIsDisable, 6000);//warte bis livebildanzeige beendet
		m_WaitLiveImageViewIsDisable.unlock();
	}
	if (m_EventTerminateThread)
	{
		CloseHandle(m_EventTerminateThread);
		m_EventTerminateThread = NULL;
	}
}

void MeasureTaskPrintCheck::DetachSharedMemoryInPrintchecktask()
{
	m_SharedMemoryFormatImage->CloseSharedMemory();
}

//Thread für die Druckkontrolle
void MeasureTaskPrintCheck::run()
{
	QString ErrorMsg;
	int rv = ERROR_CODE_NO_ERROR;
	int CounterTeachedFormats = 0;
	unsigned char *pVideoData = NULL;
	VideoHeader videoHeader;
	MeasurementResult results;
	HalconCpp::HImage  InputImage;
	int W,H,NumberChannels = 1;
	

	while (!m_TerminateInspection)
	{
		if (GetImageData() == NULL)break;//ohne Instanz von ImageData kein Messen möglich
		rv = WaitForNewImageInSharedMemory();//warte auf neues Formatbild
		if (m_TerminateInspection)break;//app soll beendet werden
		if (rv == WAIT_OBJECT_0)
		{//neues Formatbild im Speicher
			if (m_SharedMemoryFormatImage->OpenSharedMemory(ErrorMsg) == ERROR_CODE_NO_ERROR)//check is attached
			{//Shared Memory ist geöffnet(Attached)
					m_SharedMemoryFormatImage->lock();
					memcpy(&videoHeader, m_SharedMemoryFormatImage->GetSharedMemoryStartPointer(), sizeof(VideoHeader)); //Lese Bildheader
					m_ResultsFormatImage          = videoHeader.m_InspectionWindowFormatResults;//Ergebnisse der Formatbestimmung
					m_TimeStampFormatImageInMuSec = videoHeader.m_TimeStampFormatImage;//Zeitstempel des Formatbildes
					m_FormatImageWidth            = videoHeader.m_ImageWidth;//Formatbildgroesse
					m_FormatImageHeight           = videoHeader.m_ImageHeight;
					m_HoseMiddlePosition          = videoHeader.m_HoseMiddlePosition;
				    pVideoData = m_SharedMemoryFormatImage->GetSharedMemoryStartPointer();
					pVideoData = pVideoData + sizeof(VideoHeader);// +(videoHeader.m_BeginLastSubFrameIndex * videoHeader.m_ImageBlockSize);
					try
					{//Formatbild nach halcon copieren
						InputImage.GenImage1("byte", static_cast<Hlong>(m_FormatImageWidth), static_cast<Hlong>(m_FormatImageHeight), (void*)(pVideoData));
						m_HalconRGBResultImage.GenImage3("byte", m_FormatImageWidth, m_FormatImageHeight, (void*)(pVideoData), (void*)(pVideoData), (void*)(pVideoData));
						m_HalconRGBResultImage = m_HalconRGBResultImage.ZoomImageFactor(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage(), "constant");
					}
					catch (HalconCpp::HException &exception)
					{
						ErrorMsg = tr("Error Can Not Read Image Into Halcon System. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
						rv = ERROR_CODE_ANY_ERROR;
						emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
					}
					m_SharedMemoryFormatImage->unlock();
					if (rv == ERROR_CODE_NO_ERROR)
					{
						GetImageData()->IncrementFormatCounter();
						W = m_FormatImageWidth;
						H = m_FormatImageHeight;
						GetImageData()->GetDisplayZoomedSizeFormatImage(W, H);
						m_FormatImageAndResults.m_Image              = CopyHalconImageIntoQtImage(InputImage, NumberChannels);//nur für die Bildanzeige
						m_FormatImageAndResults.m_Image              = m_FormatImageAndResults.m_Image.scaled(W, H);//bild skaliert anzeigen
						m_FormatImageAndResults.m_HoseMiddlePosition = m_HoseMiddlePosition;
						if (GetImageData()->IsResumeMeasuring())
						{
							StartPrintCheck(InputImage, m_FormatImageWidth, m_FormatImageHeight);//eigentliche Druckkontrolle
						}
						emit SignalShowLiveImage(m_FormatImageAndResults);//Formatbild an die GUI
						GetImageData()->StopTimerMeasureFullInspectionTime();//Bestimmung der gesamten Inspektionzeit
    				}
			}
			else
			{
				msleep(500);
				emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
			}
		}
		else
		{//timeout kein bild, tritt hier niemals auf, weil WaitForMultipleObjects(2, hEvents, false, INFINITE) unbegrenzt wartet
			if (rv == WAIT_TIMEOUT)
			{
				ErrorMsg = tr("Timeout No Image Check Print. CameraIndex:%1").arg(GetImageData()->GetCameraIndex());
				emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
			}
		}
    }
	m_WaitConditionLiveViewIsDisable.wakeAll();
}



void MeasureTaskPrintCheck::StartInspection()
{
	start();
}


int MeasureTaskPrintCheck::WaitForNewImageInSharedMemory()
{
	int rv= ERROR_CODE_NO_ERROR;
	HANDLE  hEvents[2];

	hEvents[0] = m_SharedMemoryFormatImage->GetEventIDNewDataInSharedMemory();
	hEvents[1] = m_EventTerminateThread;
	
	rv = WaitForMultipleObjects(2, hEvents, false, INFINITE);
	if (rv == WAIT_OBJECT_0 + 1)
		m_TerminateInspection = true;//Application will be closed
	return rv;
}


void MeasureTaskPrintCheck::StartPrintCheck(HalconCpp::HImage  &InputImage,int W, int H)
{
	int rv = ERROR_CODE_NO_ERROR;
	QString Text,ErrorMsg;
	double CalculatetFormatLenght=0.0;

	if (GetImageData() && GetImageData()->GetMatchingShapeBased())
	{
		if (GetImageData()->GetNumberInspectionWindows() > 0 && m_EnableMeasuring)
		{
			QElapsedTimer timer;
			QElapsedTimer timerSubInspection;
			bool   AnyDefectFound = false;
			int NumberChannels = 1;
			double XRightBotPos = m_ResultsFormatImage.m_Results.m_ResultXPos +  (m_ResultsFormatImage.m_ModelWidthReference / 2.0);
			double XLeftTopPos  = m_ResultsFormatImage.m_Results.m_ResultXPos -  (m_ResultsFormatImage.m_ModelWidthReference / 2.0);
			double YRightBotPos = m_ResultsFormatImage.m_Results.m_ResultYPos +  (m_ResultsFormatImage.m_ModelHeightReference / 2.0);
			double YLeftTopPos  = m_ResultsFormatImage.m_Results.m_ResultYPos -  (m_ResultsFormatImage.m_ModelHeightReference / 2.0);
			
			timer.start();
			//Formatbild ausschneiden
			try
			{
				if (YLeftTopPos < 0)
					YLeftTopPos = 0;
				if (XLeftTopPos < 0)
					XLeftTopPos = 0;
				if (YRightBotPos > H)
					YRightBotPos = H - 1;
				if (XRightBotPos > W)
					XRightBotPos = W - 1;

				InputImage = InputImage.CropRectangle1(YLeftTopPos, XLeftTopPos, YRightBotPos - 1, XRightBotPos - 1);
			}
			catch (HalconCpp::HException &exception)
			{
				ErrorMsg = tr("Can Not Cut FormatImage. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
				emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
				return;
			}
			int num=GetImageData()->GetNumberInspectionWindows();
			for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
			{
				    InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
					if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
					{ //
						timerSubInspection.start();
						pInspectionWindow->m_Results.m_Time = QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz");
						pInspectionWindow->m_Results.m_Date = QDateTime::currentDateTime().date().toString("dd.MM.yy");
						pInspectionWindow->ClearResults();

						if (pInspectionWindow->m_CheckOnlyHorizontalLines)
						{//hier kontrolle Zeilenausfall
							rv = GetImageData()->GetPrintLineDetector()->StartDetection(InputImage, pInspectionWindow, ErrorMsg);
							if (rv != ERROR_CODE_NO_ERROR)
								emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
							else
							{
								pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos + XLeftTopPos;
								pInspectionWindow->m_Results.m_ResultYPos = pInspectionWindow->m_Results.m_ResultYPos + YLeftTopPos;
							}
						}
						else
						{
							rv = GetImageData()->GetMatchingShapeBased()->StartDetection(InputImage, pInspectionWindow, ErrorMsg);
							if (rv != ERROR_CODE_NO_ERROR)
								emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
							else
							{
								pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos + XLeftTopPos;
								pInspectionWindow->m_Results.m_ResultYPos = pInspectionWindow->m_Results.m_ResultYPos + YLeftTopPos;
							}
						}
						pInspectionWindow->m_Results.m_InspectionTimeInms = timerSubInspection.nsecsElapsed() / 1000000.0;
					}
			}
			AnyDefectFound = CalculateFormatResults();
			if (AnyDefectFound)
			{
					NumberChannels = 3;
					m_SubFormatErrorImageAndResults.m_Image = GetImageData()->CopyHalconImageIntoQtImage(m_HalconRGBResultImage, NumberChannels);
					emit SignalShowErrorImage(m_SubFormatErrorImageAndResults);
			}
			else
			{
				InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(INSPECTION_ID_FORMAT_WINDOW);
				if (pInspectionWindow)
				{
					CalculatetFormatLenght=pInspectionWindow->m_Results.m_FormatLengthInPix*GetImageData()->GetPixelSize();
					if (CalculatetFormatLenght > 0.0)
					{
						Text = tr("Format Ok(%1mm)").arg(CalculatetFormatLenght, 0, 'f', 2);
						GetImageData()->ShowInspectionErrorText(Text, false);
					}
				}
    		}
			if (GetImageData()->GetSubFormatData())
				GetImageData()->GetSubFormatData()->m_ResultInspectionTimeInMs = timer.nsecsElapsed() / 1000000.0;
		}
		else
		{
			ErrorMsg = tr("Can Not Measure No Reference Data Available");
			emit SignalShowMessage(ErrorMsg, QtMsgType::QtCriticalMsg);
		}
    }
}


void MeasureTaskPrintCheck::OverPaintDefectRegion(InspectionWindow *pInspectionWindow)
{
	HalconCpp::HTuple RGB, HomMat2D, Row(pInspectionWindow->m_Results.m_ResultYPos), Column(pInspectionWindow->m_Results.m_ResultXPos), Angle(pInspectionWindow->m_Results.m_ModelAngle*RAD_PER_DEGREE);;
	HalconCpp::HRegion  ZoomedRegion;
	double ModelWidth2  = pInspectionWindow->m_ModelWidthReference / 2.0;
	double ModelHeight2 = pInspectionWindow->m_ModelHeightReference / 2.0;

	HomMat2dIdentity(&HomMat2D);//einheitsmatrix erzeugen
	HomMat2dRotate(HomMat2D, Angle, ModelHeight2, ModelWidth2, &HomMat2D);//rotationsmatrix
	Row    = Row    - ModelHeight2;
	Column = Column - ModelWidth2;
	HomMat2dTranslate(HomMat2D, Row, Column, &HomMat2D);//translation
	AffineTransRegion(pInspectionWindow->m_ResultRegionDiff, &(pInspectionWindow->m_ResultRegionDiff), HomMat2D, HalconCpp::HTuple("nearest_neighbor"));//drehung und verschiebung der defektregion auf die aktuelle position
	
	ZoomedRegion = pInspectionWindow->m_ResultRegionDiff.ZoomRegion(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage());
	RGB.Append(255);//Fehlstellen Rot einzeichnen
	RGB.Append(0);
	RGB.Append(0);
	OverpaintRegion(*GetImageData()->GetHalconRGBResultImage(), ZoomedRegion, RGB, HalconCpp::HTuple("fill"));
}


void MeasureTaskPrintCheck::OverPaintMiddleLine()
{
	HalconCpp::HRegion  LineRegion, ZoomedRegion;
	HalconCpp::HTuple RGB;

	RGB.Append(0);
	RGB.Append(0);
	RGB.Append(255);
	LineRegion.GenRegionLine(static_cast<Hlong>(m_HoseMiddlePosition+0.5), 0, static_cast<Hlong>(m_HoseMiddlePosition + 0.5), static_cast<Hlong>(m_FormatImageWidth));
	ZoomedRegion = LineRegion.ZoomRegion(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage());
	OverpaintRegion(*GetImageData()->GetHalconRGBResultImage(), ZoomedRegion, RGB, HalconCpp::HTuple("fill"));
}


void MeasureTaskPrintCheck::OverPaintYOffset(int YOffset,int Xpos,int ModelLenght)
{
	HalconCpp::HRegion  LineRegion, ZoomedRegion;
	HalconCpp::HTuple RGB;

	RGB.Append(255);//Line Rot einzeichnen
	RGB.Append(0);
	RGB.Append(0);
	LineRegion.GenRegionLine(static_cast<Hlong>(YOffset), static_cast<Hlong>(Xpos - ModelLenght / 2.0), static_cast<Hlong>(YOffset), static_cast<Hlong>(Xpos+ ModelLenght/2.0));
	ZoomedRegion = LineRegion.ZoomRegion(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage());
	OverpaintRegion(*GetImageData()->GetHalconRGBResultImage(), ZoomedRegion, RGB, HalconCpp::HTuple("fill"));
}


void MeasureTaskPrintCheck::OverPaintModelNotFound(int xpos, int ypos, int ModelWidth,int ModelHeight)
{
	HalconCpp::HRegion  RegionRectangle, ZoomedRegion;
	HalconCpp::HTuple RGB;
	int ModelWidth2  = static_cast<int>(ModelWidth / 2.0);
	int ModelHeight2 = static_cast<int>(ModelHeight / 2.0);

	RGB.Append(255);
	RGB.Append(0);
	RGB.Append(0);
	RegionRectangle.GenRectangle1(static_cast<Hlong>(ypos), static_cast<Hlong>(xpos), static_cast<Hlong>(ypos + ModelHeight), static_cast<Hlong>(xpos + ModelWidth));
	ZoomedRegion = RegionRectangle.ZoomRegion(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage());
	OverpaintRegion(*GetImageData()->GetHalconRGBResultImage(), ZoomedRegion, RGB, HalconCpp::HTuple("margin"));
}


bool MeasureTaskPrintCheck::CalculateFormatResults()
{
	bool AnyError = false;
	SubFormatData *pFormatData = NULL;
	double MeanDefectScore = 0.0;
	double MaxDefectScore = 0.0;
	double MeanOffsetInY = 0.0;
	double MaxOffsetInY = 0.0;
	double YOffsetInMM;
	double XLeftTopPos,YLeftTopPos;
	int CountValidResults=0;
	double PixelSize                      = GetImageData()->GetPixelSize();
	QList< ImageLineInformation> ListLineInformation;
	bool error_font     = false;
	bool error_position = false;
	bool error_line     = false;
	bool error_height   = false;
	bool error_width    = false;
	InspectionWindow ErrorInspectionRect;
	InspectionWindow CurrenTInspectRect;
	QString ErrorText;
    
	//Enthält nur die Inspektionsfehler für die Bildanzeige/Klasse ErrorImageView
	m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.clear();//fehlerbild mit messergebnissen
	m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.append(m_ResultsFormatImage);//Daten für die Bildanzeige
	//Enthält alle Messergebnisse für die Bildanzeige/Klasse LiveImageView
	m_FormatImageAndResults.m_ListInspectionWindowResults.clear();//ergebnisse mit dem Livebild an die GUI 
	m_FormatImageAndResults.m_ListInspectionWindowResults.append(m_ResultsFormatImage);

	GetImageData()->GenerateImageTimeStampList(ListLineInformation, m_FormatImageWidth, m_TimeStampFormatImageInMuSec);//Liste der Fehler pro Kameraspalte. Hier Kameraspalte da Bild um 90 Grad gedreht
	pFormatData = GetImageData()->GetSubFormatData();
	if (pFormatData && GetImageData()->GetNumberInspectionWindows()>0)
	{
		for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
		{
			InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
			if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID != INSPECTION_ID_FORMAT_WINDOW)
			{
				if (pInspectionWindow->m_Results.m_ResultsValid)//sind die Messergebnissegültig
				{//Messung erfolgreich
					if (!pInspectionWindow->m_CheckOnlyHorizontalLines)
					{
						if (pInspectionWindow->m_Results.m_ModelFound)
						{//wurde das Objekt gefunden
							CountValidResults++;
							MeanDefectScore = MeanDefectScore + pInspectionWindow->m_Results.m_DefectScore;
							if (pInspectionWindow->m_Results.m_DefectScore > MaxDefectScore)
								MaxDefectScore = pInspectionWindow->m_Results.m_DefectScore;
							if (pInspectionWindow->m_Results.m_DefectScore > pInspectionWindow->m_PrintErrorTolInPercent)//GetImageData()->GetPrintErrorTolInPercent())//ist druck ok?
							{
								pInspectionWindow->m_Results.m_PrintOk = false;
								if(pInspectionWindow->m_MeasureVarianteDiffImage)
								   OverPaintDefectRegion(pInspectionWindow);//Fehlerstellen in das Fehlerbild einzeichnen
								if (pInspectionWindow->m_EnableInspection)
								    error_font = true;
								AnyError = true;
								ErrorInspectionRect = *pInspectionWindow;
								m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.append(ErrorInspectionRect);
								ErrorText = tr("Print Not Ok! Name:%1").arg(pInspectionWindow->m_ModelName);
								GetImageData()->ShowInspectionErrorText(ErrorText,true);
                                if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                                {
                                    if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                        qDebug() << " font err top";
                                    else
                                        qDebug() << " font err bot";
                                }
							}
							else
							{//wenn Druck Ok, dann Prüfung ob Position Ok
								pInspectionWindow->m_Results.m_PrintOk = true;
								pInspectionWindow->m_Results.m_OffsetY = fabs(pInspectionWindow->m_Results.m_ResultYPos - m_HoseMiddlePosition);
								MeanOffsetInY = MeanOffsetInY + pInspectionWindow->m_Results.m_OffsetY;
								if (pInspectionWindow->m_Results.m_OffsetY > MaxOffsetInY)
									MaxOffsetInY = pInspectionWindow->m_Results.m_OffsetY;
								YOffsetInMM = pInspectionWindow->m_Results.m_OffsetY * PixelSize;
								if (YOffsetInMM > GetImageData()->GetPositionTolInMM())
								{
									pInspectionWindow->m_Results.m_PositionOk = false;
									if (pInspectionWindow->m_EnableInspection)
									    error_position = true;
									AnyError = true;
									ErrorInspectionRect = *pInspectionWindow;
									m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.append(ErrorInspectionRect);
									ErrorText = tr("Position Not Ok! Name:%1").arg(pInspectionWindow->m_ModelName);
									GetImageData()->ShowInspectionErrorText(ErrorText,true);
									OverPaintYOffset(pInspectionWindow->m_Results.m_ResultYPos, pInspectionWindow->m_Results.m_ResultXPos, pInspectionWindow->m_ModelWidthReference);
                                    if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                                    {
                                        if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                            qDebug() << " pos err top";
                                        else
                                            qDebug() << " pos err bot";
                                    }
								}
								else
								{//Wenn Position Ok, dann Prüfung liegt ein Längenfehler vor
									pInspectionWindow->m_Results.m_PositionOk = true;
									double DeltaXInMM = (pInspectionWindow->m_Results.m_ObjectSizeInX - pInspectionWindow->m_ModelWidthReference)   * GetImageData()->GetPixelSize();
									double DeltaYInMM = (pInspectionWindow->m_Results.m_ObjectSizeInY - pInspectionWindow->m_ModelHeightReference)  * GetImageData()->GetPixelSize();
									if (fabs(DeltaXInMM) > GetImageData()->GetBlockWidthTolInMM())
									{
										pInspectionWindow->m_Results.m_BlockWidthOk = false;
										pInspectionWindow->m_Results.m_LenghtErrorInMM = DeltaXInMM;
										if (pInspectionWindow->m_EnableInspection)
										    error_width = true;
										AnyError = true;
										ErrorText = tr("%1 Lenght Not Ok ").arg(pInspectionWindow->m_ModelName);
                                        if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                                        {
                                            if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                                qDebug() <<  " w err top";
                                            else
                                                qDebug() <<  " w err bot";
                                        }
									}
									else
										pInspectionWindow->m_Results.m_BlockWidthOk = true;
									if (fabs(DeltaYInMM) > GetImageData()->GetBlockHeightTolInMM())
									{
										pInspectionWindow->m_Results.m_BlockHeightOk = false;
										pInspectionWindow->m_Results.m_HeightErrorInMM = DeltaYInMM;
										if (pInspectionWindow->m_EnableInspection)
										   error_height = true;
										AnyError = true;
                                        QString temText = tr("%1 Height Not Ok").arg(pInspectionWindow->m_ModelName);
										ErrorText += temText;
                                        if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                                        {
                                            if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                                qDebug() << " h err top";
                                            else
                                                qDebug() << " h err bot";
                                        }
									}
									else
										pInspectionWindow->m_Results.m_BlockHeightOk = true;
									if (error_width || error_height)
									{
										GetImageData()->ShowInspectionErrorText(ErrorText,true);
										ErrorInspectionRect = *pInspectionWindow;
										m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.append(ErrorInspectionRect);
									}
								}
							}
						}
						else
						{//Model not found
							XLeftTopPos = m_ResultsFormatImage.m_Results.m_ResultXPos - (m_ResultsFormatImage.m_Results.m_ObjectSizeInX / 2.0);
							YLeftTopPos = m_ResultsFormatImage.m_Results.m_ResultYPos - (m_ResultsFormatImage.m_Results.m_ObjectSizeInY / 2.0);
							if (pInspectionWindow->m_EnableInspection)
							    error_font = true;
							AnyError = true;
							ErrorInspectionRect = *pInspectionWindow;
							m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.append(ErrorInspectionRect);
							ErrorText = tr("Model Not Found! Name:%1").arg(pInspectionWindow->m_ModelName);
							GetImageData()->ShowInspectionErrorText(ErrorText,true);
							OverPaintModelNotFound(XLeftTopPos + pInspectionWindow->m_ROIRectRelatetToFormatRect.x(), YLeftTopPos + pInspectionWindow->m_ROIRectRelatetToFormatRect.y(), pInspectionWindow->m_ROIRectRelatetToFormatRect.width(), pInspectionWindow->m_ROIRectRelatetToFormatRect.height());
                            if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                            {
                                if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                    qDebug() <<  " model not found top";
                                else
                                    qDebug() <<  " model not found bot";
                            }
						}
					}
					else
					{//here check only line missing
						if (!pInspectionWindow->m_Results.m_LineCheckOk)
						{
							XLeftTopPos = m_ResultsFormatImage.m_Results.m_ResultXPos - (m_ResultsFormatImage.m_Results.m_ObjectSizeInX / 2.0);
							YLeftTopPos = m_ResultsFormatImage.m_Results.m_ResultYPos - (m_ResultsFormatImage.m_Results.m_ObjectSizeInY / 2.0);
							if (pInspectionWindow->m_EnableInspection)
							    error_line = true;
							AnyError = true;
							ErrorInspectionRect = *pInspectionWindow;
							m_SubFormatErrorImageAndResults.m_ListInspectionWindowResults.append(ErrorInspectionRect);
							ErrorText = tr("Line Missing! Name:%1").arg(pInspectionWindow->m_ModelName);
							GetImageData()->ShowInspectionErrorText(ErrorText,true);
							OverPaintModelNotFound(XLeftTopPos + pInspectionWindow->m_ROIRectRelatetToFormatRect.x(), YLeftTopPos + pInspectionWindow->m_ROIRectRelatetToFormatRect.y(), pInspectionWindow->m_ROIRectRelatetToFormatRect.width(), pInspectionWindow->m_ROIRectRelatetToFormatRect.height());
                            if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                            {
                                if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                    qDebug() << " line err top";
                                else
                                    qDebug() << " line err bot";
                            }
    					}
					}
					CurrenTInspectRect = *pInspectionWindow;
					m_FormatImageAndResults.m_ListInspectionWindowResults.append(CurrenTInspectRect);//enthält alle Messergebnisse für die Bildanzeige für die Klasse LiveImageView
				}//if (pInspectionWindow->m_Results.m_ResultsValid)
			}
		}
		if (CountValidResults > 0)
		{
			pFormatData->m_ResultMeanDefectScore = MeanDefectScore / CountValidResults;
			pFormatData->m_ResultMeanOffsetInY   = MeanOffsetInY   / CountValidResults;
		}
		pFormatData->m_ResultMaxDefectScore = MaxDefectScore;
		pFormatData->m_ResultMaxOffsetInY   = MaxOffsetInY;
		if (AnyError)
			OverPaintMiddleLine();
		for (int i = 0; i < ListLineInformation.count(); i++)
		{
			ListLineInformation[i].error_font     = error_font;
			ListLineInformation[i].error_line     = error_line;
			ListLineInformation[i].error_position = error_position;
			ListLineInformation[i].error_height   = error_height;
			ListLineInformation[i].error_width    = error_width;
		}
		//jetzt die Ergebnisse an die SPS bzw erst in die MainLogic Klasse. 
		if (GetImageData()->GetMainAppPrintCheck()->GetPLC())
			GetImageData()->GetMainAppPrintCheck()->GetPLC()->AppendLineInformation(ListLineInformation, GetImageData()->GetCameraIndex());
	}
	return AnyError;
}


QImage MeasureTaskPrintCheck::CopyHalconImageIntoQtImage(HalconCpp::HImage  &HalconImage, int NumberChannelsQtImage)
{
	HalconCpp::HString    HType;
	HalconCpp::HTuple NumberChannelsHalconImage, Type, W, H, S, R, G, B;
	int idx;
	unsigned char *s, *r, *g, *b;
	long size, w, h;
	QRgb *pixels;
	QImage QtImage;


	CountChannels(HalconImage, &(NumberChannelsHalconImage));
	if (NumberChannelsHalconImage == 1)
		GetImagePointer1(HalconImage, &S, &Type, &W, &H);
	else
		GetImagePointer3(HalconImage, &R, &G, &B, &Type, &W, &H);
	w = (Hlong)(W);
	h = (Hlong)(H);
	size = w * h;

	if (NumberChannelsQtImage == 1)
	{
		QtImage = QImage(w, h, QImage::Format_Indexed8);
		QtImage.setColorTable(GetColorTable());
	}
	else
		QtImage = QImage(w, h, QImage::Format_ARGB32);

	if (NumberChannelsHalconImage == 1 && NumberChannelsQtImage == 1)
	{//grauwertbild
		s = (unsigned char*)(S.L());//(Image.m_HalconImage->GetImagePointer1(&typ,&width,&height));
		for (int y = 0; y < h; y++)
		{
			memcpy(QtImage.scanLine(y), s, w);
			s = s + w;
		}
	}
	else
		if (NumberChannelsHalconImage == 1 && NumberChannelsQtImage == 3)
		{//grauwertbild -> farbbild
			pixels = reinterpret_cast<QRgb*>(QtImage.bits());
			s = (unsigned char *)(S.L());
			for (idx = 0; idx < size; idx++)
			{
				pixels[idx] = qRgb(*s, *s, *s);
				s++;
			}
		}
		else
			if (NumberChannelsHalconImage == 3 && NumberChannelsQtImage == 3)
			{//farbbild copieren
				pixels = reinterpret_cast<QRgb*>(QtImage.bits());
				r = (unsigned char *)(R.L());
				g = (unsigned char *)(G.L());
				b = (unsigned char *)(B.L());
				for (idx = 0; idx < size; idx++)
				{
					pixels[idx] = qRgb(*r, *g, *b);
					r++;
					g++;
					b++;
				}
			}
	return QtImage;
}






