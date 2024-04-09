#include "MeasureTaskFormatCheck.h"
#include "ImageData.h"
#include "SharedMemoryVideoData.h"
#include "GlobalConst.h"
#include "VideoHeader.h"
#include "MatchingShapeBased.h"
#include "MeasureTaskDetectHose.h"
#include "MainAppPrintCheck.h"
#include "MainLogic.h"



MeasureTaskFormatCheck::MeasureTaskFormatCheck(ImageData *pImageData) : QThread()
, m_ImageData(NULL)//Paren tClass
, m_SharedMemoryFullHose(NULL)//Bildspeicher für kompletten Schlauch
, m_SharedMemoryFormatImage(NULL)//Bildspeicher für das Format, die klasse MeasuretaskprintCheck liest das Formatbild aus diesem Speicher und führt die Druckkontrolle aus
, m_EventTerminateThread(NULL)//wenn Programm beendet, dann wird der Thread aus dem Wartezustand terminiert
, m_ROIRimInPixel(16)//Bildrandbereich für das Format
, m_TerminateInspection(false)//wenn auf true, dann wird der Laufende Thread beendet
, m_ReadDirectionLeftToRight(true)//Flag ob das Formatbild gespiegelt wird
, m_FormatNotFoundCounter(0)//Zählt wie oft hintereinander das Format nicht gefunden wurde
, m_LastXPosInHoseCoordinates(0.0)//letzte Xpos des Formates im Bildspeicher
{
	m_EventTerminateThread = CreateEventW(NULL, FALSE, FALSE, NULL);//thread aus dem wartezustand bringen und dann terminieren
	m_ImageData = pImageData;
	
	m_SharedMemoryFullHose = new SharedMemoryVideoData();//Bildspeicher für kompletten Schlauch
	m_SharedMemoryFullHose->SetKeyName(QString("%1%2").arg(SHARED_MEMORY_KEYNAME_FULL_HOSE_IMAGE).arg(GetImageData()->GetCameraIndex() + 1));

	m_SharedMemoryFormatImage = new SharedMemoryVideoData();//Bildspeicher für ein Format
	m_SharedMemoryFormatImage->SetKeyName(QString("%1%2").arg(SHARED_MEMORY_KEYNAME_FORMAT_IMAGE).arg(GetImageData()->GetCameraIndex() + 1));
}


MeasureTaskFormatCheck::~MeasureTaskFormatCheck()
{
	WaitForFinshed();//Thread beenden
	if (m_SharedMemoryFullHose)//Speicher freigeben
	{
		delete m_SharedMemoryFullHose;
		m_SharedMemoryFullHose = NULL;
	}
	if (m_SharedMemoryFormatImage)//Speicher freigeben
	{
		delete m_SharedMemoryFormatImage;
		m_SharedMemoryFormatImage = NULL;
	}
}

//laufenden Thread beenden
void MeasureTaskFormatCheck::WaitForFinshed()
{
	if (isRunning())
	{//thread läuft noch
		if (m_EventTerminateThread)
			SetEvent(m_EventTerminateThread);//wenn er im Wartezustand, dann hiermit beenden
		m_TerminateInspection = true;
		m_MutexThreadIsFinished.lock();
		m_WaitConditionThreadIsFinished.wait(&m_MutexThreadIsFinished, 2000);
		m_MutexThreadIsFinished.unlock();
	}
	if (m_EventTerminateThread)
	{
		CloseHandle(m_EventTerminateThread);
		m_EventTerminateThread = NULL;
	}
}

//warte auf neues Bild von der Task MeasureTaskDetectHose
int MeasureTaskFormatCheck::WaitForNewImageInSharedMemory()
{
	int rv = ERROR_CODE_NO_ERROR;
	HANDLE  hEvents[2];

	hEvents[0] = m_SharedMemoryFullHose->GetEventIDNewDataInSharedMemory();
	hEvents[1] = m_EventTerminateThread;//wird auf signalisierend gesetzt wenn Programm beendet

	rv = WaitForMultipleObjects(2, hEvents, false, INFINITE);
	if (rv == WAIT_OBJECT_0 + 1)
		m_TerminateInspection = true;//Application will be closed
	return rv;
}

//Task suche Format
void MeasureTaskFormatCheck::run()
{
	int rv = ERROR_CODE_NO_ERROR;
	QString ErrorMsg;
	InspectionWindow *pInspectionWindow = NULL;//Messfenster Formatblock
	int InspectionWindowIDFormatRect = 0;//index für das Formatfenster

	while (!m_TerminateInspection)
	{
		if (GetImageData() == NULL)break;//ohne Instanz von ImageData kein Messen möglich
		rv = WaitForNewImageInSharedMemory();//warte auf neues Bild
		if (m_TerminateInspection)break;//app soll beendet werden
		if (rv == WAIT_OBJECT_0)
		{//neues bild im Speicher
			if (m_SharedMemoryFullHose->OpenSharedMemory(ErrorMsg) == ERROR_CODE_NO_ERROR)//check is attached
			{//Shared Memory ist geöffnet(Attached)
				if (GetImageData()->IsResumeMeasuring())
				{//startmeasuring is active
					pInspectionWindow = GetImageData()->GetInspectionWindowByID(InspectionWindowIDFormatRect);
					if (pInspectionWindow)
					{
						if (!GetImageData()->GetCheckNewReference())
						{//normales messen Hier beginnt die eigentliche Messung
							if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)//Prüfe Orientierung der Kamera
								m_ReadDirectionLeftToRight = !(GetImageData()->GetTopCameraIsFirst());
							else
								m_ReadDirectionLeftToRight = (GetImageData()->GetTopCameraIsFirst());
							GetImageData()->StartTimerMeasureFullInspectionTime();//Starte Timer für die Inspektionzeit
							rv = CheckFormat(pInspectionWindow, ErrorMsg);//Prüfung ob ein Format vollständig im Speicher(m_SharedMemoryFormatImage) 
							if (rv == ERROR_CODE_ANY_ERROR)
								emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);//Schwerwiegender Fehler die Messung konnte nich durchgeführt werden
						}
						else
						{
							msleep(500);//Ein neues Refernzbild wird gerade aufgenommen
						}
					}
				}
			}
			else
			{
				msleep(500);//Problem mit dem Anlegen des SharedMemory
				emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
			}
		}
	}
	m_WaitConditionThreadIsFinished.wakeAll();//Signal and die GUI Thread ist beendet
}

//Suche Format im aktuellen Bild
int MeasureTaskFormatCheck::CheckFormat(InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	int ImageStartPosNewFormat;//Bildindex im SharedMemory bei dem das Letzte Format gefunden wurde bzw. das Ende
	int CurrentFrameImageHeight;//Aktuelle Bildgröße indem das Format gesucht wird
	int NumCameraImagesIncludesOneFormat;//Anzahl der Kamerabilder die vorhanden sein müssen damit ein Format vollständig enthalten ist
	int NumberCameraImagesInBuffer;//Aktuelle Anzahl von Kamerbildern die gerede verarbeitet werden 
	int NumberFormatsNotFound = 0;//Zähler wie oft hintereinander kein Format gefunden wurde
	int CameraImageOffset = 0;//Offset auf die neue ImageStartPosNewFormat für den nächsten durchlauf
	int NumberImagesPerFormat = static_cast<int>(GetImageData()->GetFormatLenghtInPixel() / GetImageData()->GetCameraHeightInPixel() + 0.5);//aus wievielen Kamerblöcken besteht ein Format(Ganzzahlig)
	int NumberLinesIncludesOneFormat = GetImageData()->GetFormatLenghtInPixel() * 2 + 64;//Anzahl Bildzeilen von der Kamera indem ein Format vollständig enthalten ist
	int OverlapValue = 16;
	unsigned char *pShareData = NULL;//Bildstartposition im SharedMemory
	double XTopLeft, YTopLeft, XBottomRight, YBottomRight;
	double XRightPos, XLeftPos;
	double HoseMiddlePosition;//Schlauch Mittenposition
	bool FormatFound = false;
	bool FormatMissing;
	unsigned __int64 ImageTimeStampInMuSec;//Zeitstempel aktuelles Bild
	QList<ImageLineInformation>  ListLineInformation;//Ergebnissliste für die SPS
	MeasurementResult            CurrentAverageResultsHoseDetection;//Messergebnisse von der Schlaucherkennung. Schlauchposition und Schlauchbreite
	InspectionWindow             ResultsToGUI;//Messergebnisse an die GUI Bildanzeige
	QElapsedTimer                MeasureInspectionTimeFoundFormat;//Messzeit für die Formatsuche
	Hlong W, H;//Formatbildgröße
	HalconCpp::HImage  FormatImage;//Formatbild

	
	if (GetImageData())
	{ 
	  m_SharedMemoryFullHose->lock();
	  ImageTimeStampInMuSec                 = GetImageData()->GetMeasureTaskDetectHose()->GetImageTimeStampByImageIndex(GetImageData()->GetMeasureTaskDetectHose()->GetNumberImagesInSharedMemory() - 1);//Zeitstempel des letzten Bildes im SharedMemory
	  ImageStartPosNewFormat                = GetImageData()->GetMeasureTaskDetectHose()->GetSharedMemoryHeaderHoseImageStartPosNewFormat();//Bildindex im SharedMemory 
	  NumCameraImagesIncludesOneFormat      = (NumberImagesPerFormat * 2) + 1;//Anzahl der Kamerabilder bei dem sichergestellt ist das mindestens ein Format vollständig enthalten ist
	  NumberCameraImagesInBuffer            = GetImageData()->GetMeasureTaskDetectHose()->GetNumberImagesInSharedMemory() - ImageStartPosNewFormat;//Aktuelle Anzahl Bilder die im Speicher bei dem noch kein Format gefunden wurde
	  if(NumberCameraImagesInBuffer>0)
	  {
		  CurrentFrameImageHeight =  NumberCameraImagesInBuffer * GetImageData()->GetCameraHeightInPixel();
		  if (CurrentFrameImageHeight > NumberLinesIncludesOneFormat)
			  CurrentFrameImageHeight = NumberLinesIncludesOneFormat;//exakte Bildhöhe indem mindesten ein Format vollständig enthalten sein muß
		  //Schlauch gefunden für die SPS auf true setzen
		  GetImageData()->GenerateImageTimeStampList(ListLineInformation, CurrentFrameImageHeight, ImageTimeStampInMuSec);//erzeugung der TimeStamps füe jede Bildzeile
		  for (int i = 0; i < ListLineInformation.count(); i++)//hier landen nur Bilder wenn Schlauch gefunden, daher hier and die SPS bzw die MainLogic Klasse Schlauch gefunden auf true
			  ListLineInformation[i].tube_found = true;
		  
		  pShareData = m_SharedMemoryFullHose->GetSharedMemoryStartPointer() + (ImageStartPosNewFormat * GetImageData()->GetCameraWidthInPixel() * GetImageData()->GetCameraHeightInPixel());
		  if (pShareData)
		  {//Bild aus dem Sharedmemory in ein Halconbild speichern
				  try
				  {
					  FormatImage.GenImage1("byte", static_cast<Hlong>(GetImageData()->GetCameraWidthInPixel()), static_cast<Hlong>(CurrentFrameImageHeight), (void*)(pShareData));
				  }
				  catch (HalconCpp::HException &exception)
				  {//Bild kann nicht kopiert werden
					  ErrorMsg = tr("Search Full Format Read Image. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
					  rv = ERROR_CODE_ANY_ERROR;
				  }
		  }
		  if (rv == ERROR_CODE_NO_ERROR)
		  {
				  CurrentAverageResultsHoseDetection = GetImageData()->GetMeasureTaskDetectHose()->GetCurrentResultsHoseDetection();
				  if (CurrentAverageResultsHoseDetection.m_ObjectSizeInX > 0.0 && CurrentAverageResultsHoseDetection.m_ObjectSizeInY > 0.0)
				  {//da schlauchposition und Schlauchdurchmesser bekannt, kann der Suchbereich eingeschränkt werden
					  XTopLeft     = CurrentAverageResultsHoseDetection.m_ResultXPos - (CurrentAverageResultsHoseDetection.m_ObjectSizeInX / 2.0 + m_ROIRimInPixel);
					  YTopLeft     = 0.0;
					  XBottomRight = CurrentAverageResultsHoseDetection.m_ResultXPos + (CurrentAverageResultsHoseDetection.m_ObjectSizeInX / 2.0 + m_ROIRimInPixel);
					  YBottomRight = CurrentFrameImageHeight;

					  if (XTopLeft < 0.0)
						  XTopLeft = 0.0;
					  if (XBottomRight > GetImageData()->GetCameraWidthInPixel())
						  XBottomRight = GetImageData()->GetCameraWidthInPixel();
					  FormatImage = FormatImage.CropRectangle1(YTopLeft, XTopLeft, YBottomRight - 1, XBottomRight - 1); //ROI Ausschneiden bzw. nur den Schlauch
					  HoseMiddlePosition = CurrentAverageResultsHoseDetection.m_ResultXPos - XTopLeft;//ist die Mittenposition des Schlauches in dem ausgeschnittenen Formatbild
				  }
				  FormatImage = FormatImage.RotateImage(90.0, "bilinear");//Bild Drehen damit die Schrift lesbar ist
				  if (!m_ReadDirectionLeftToRight)
					  FormatImage = FormatImage.MirrorImage("column");//Spiegeln horizontal
				  MeasureInspectionTimeFoundFormat.start();
				  pInspectionWindow->ClearResults();
			      rv = GetImageData()->GetMatchingFormatWindow()->StartDetection(FormatImage, pInspectionWindow, ErrorMsg);//Wenn Format gefunden, dann ist sichergestellt das es vollständig enthalten ist
				  if (rv == ERROR_CODE_NO_ERROR)
				  {
					  if (pInspectionWindow->m_Results.m_ModelFound)
					  {//Format gefunden, und es ist vollständig im Bild enthalten
						    
						    XRightPos = pInspectionWindow->m_Results.m_ResultXPos + (pInspectionWindow->m_ModelWidthReference / 2.0);
						    XLeftPos  = pInspectionWindow->m_Results.m_ResultXPos - (pInspectionWindow->m_ModelWidthReference / 2.0);
						    FormatImage.GetImageSize(&W, &H);
						    if (!m_ReadDirectionLeftToRight)
							{//hier sind die Bilder gespiegelt(Horizontal)
								CameraImageOffset = static_cast<int>((CurrentFrameImageHeight - (XLeftPos + OverlapValue)) / GetImageData()->GetCameraHeightInPixel());
								FormatMissing=CalculateFormatLenght(pInspectionWindow, W - pInspectionWindow->m_Results.m_ResultXPos, ImageStartPosNewFormat);//Berechnet den Abstand zwischen zwei Formaten(Formatlänge)
							}
							else
							{
								CameraImageOffset = static_cast<int>((XRightPos - OverlapValue) / GetImageData()->GetCameraHeightInPixel());
								FormatMissing=CalculateFormatLenght(pInspectionWindow, pInspectionWindow->m_Results.m_ResultXPos, ImageStartPosNewFormat);
							}
							//if (FormatMissing)
							//{
							//	QString Path = QString("d://Temp3//FormatImage.bmp");
							//	FormatImage.WriteImage("bmp", 0, Path.toLatin1().data());
							//}
							ImageStartPosNewFormat = ImageStartPosNewFormat + CameraImageOffset;
							GetImageData()->GetMeasureTaskDetectHose()->SetSharedMemoryHeaderHoseImageStartPosNewFormat(ImageStartPosNewFormat);//setze neue Bildstartposition für den nächsten Durchlauf
							FormatFound = true;
							m_FormatNotFoundCounter = 0;
							FormatImage = FormatImage.CropRectangle1(0, XLeftPos, H, XRightPos);//Das gefundene Format ausschneiden und dann in den SharedMemory um die Druckinspektion zu starten
							pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos - XLeftPos;//Messergebnis anpassen(Formatkoordinaten)
							pInspectionWindow->m_Results.m_Time = QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz");
							pInspectionWindow->m_Results.m_Date = QDateTime::currentDateTime().date().toString("dd.MM.yy");
							if (CheckIsFormatInTolerance(pInspectionWindow))
							{//Formatlänge ist innerhalb der toleranz, AddFormatImage startet dann die eigentliche Druckbildkontrolle
								AppendMatchSocoreAndAverage(pInspectionWindow);//Mittelwert des MatchScores, dient dazu die Schwellwerte richtig zu setzen, der Wert ist dann in der Anzeige im Tabellenformat sichtbar
								rv = AddFormatImage(FormatImage, pInspectionWindow, ImageTimeStampInMuSec, HoseMiddlePosition, ErrorMsg);//Formatbild in den Format Sharedmemory, die Task MeasureTaskPrintCheck prüft dann das Durckbild
							}
							else
							{//Format ist zu lang oder zu kurz, Fehler an die SPS
								ResultsToGUI = *pInspectionWindow;
								m_FormatErrorImage.m_ListInspectionWindowResults.clear();
								m_FormatErrorImage.m_ListInspectionWindowResults.append(ResultsToGUI);//daten für die Bildanzeige
								for (int i = 0; i < ListLineInformation.count(); i++)
									ListLineInformation[i].error_width = true;//Fehlercode an die SPS
								HalconCpp::HImage ErrorImage = FormatImage.ZoomImageFactor(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage(), "constant");
								m_FormatErrorImage.m_Image = GetImageData()->CopyHalconImageIntoQtImage(ErrorImage, 3);
								emit SignalShowFormatNotFoundImage(m_FormatErrorImage);//in die Fehlerbildanzeige
								GetImageData()->ShowInspectionErrorText(tr("Format Lenght Error"), true);
                                if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                                {
                                    if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                                        qDebug() << "error format lenght top";
                                    else
                                        qDebug() << "error format lenght bot";
                                }

							}
							pInspectionWindow->m_Results.m_InspectionTimeInms = MeasureInspectionTimeFoundFormat.nsecsElapsed() / 1000000.0;
					  }
					  else
					  {//bild ist groß genug aber kein druckbild vorhanden
						  rv = ERROR_CODE_FORMAT_NOT_FOUND_IN_FORMAT_ROI;
						  GetImageData()->StopTimerMeasureFullInspectionTime();
					  }
				  }
				  else
				  {//Messung konnte nicht durchgeführt werden , Gravierender Fehler
					  GetImageData()->GetMeasureTaskDetectHose()->SetNumberImagesInSharedMemory(0);
					  GetImageData()->GetMeasureTaskDetectHose()->SetSharedMemoryHeaderHoseImageStartPosNewFormat(0);
					  GetImageData()->StopTimerMeasureFullInspectionTime();
				  }
     	  }
		  if (!FormatFound)
		  {	  
			  m_FormatNotFoundCounter++;//Zählt wie oft pro Kamerablock das Format nicht gefunden wurde
			  if (m_FormatNotFoundCounter > NumCameraImagesIncludesOneFormat)
			  {//Sicherstellen, dass nur zwei direkt hintereinander liegende Formate zur Berechnung der Formatlänge berücksichtigt wereden
				  m_LastXPosInHoseCoordinates = 0.0;
			  }
			  //if (m_FormatNotFoundCounter >= ((NumCameraImagesIncludesOneFormat / 1.5)* GetImageData()->GetMainAppPrintCheck()->GetNumberEvaluationPeriodFormatNotFound()))
			  //{
			//	  for (int i = 0; i < ListLineInformation.count(); i++)//hier Format nicht gefunden für die SPS
			//		  ListLineInformation[i].no_format = true;
			//  }
			  if (m_FormatNotFoundCounter >= ((NumCameraImagesIncludesOneFormat) * GetImageData()->GetMainAppPrintCheck()->GetNumberEvaluationPeriodFormatNotFound()))
			  {
				  pInspectionWindow->m_Results.m_FormatFound = false;
				  ResultsToGUI = *pInspectionWindow;
				  m_FormatErrorImage.m_ListInspectionWindowResults.clear();
				  m_FormatErrorImage.m_ListInspectionWindowResults.append(ResultsToGUI);//daten für die Bildanzeige

				 /* if (ListLineInformation.count() >= GetImageData()->GetCameraHeightInPixel())
				  {//ersten Block auf Fehler setzen
					  for (int i = 0; i < GetImageData()->GetCameraHeightInPixel(); i++)//hier Format nicht gefunden für ersten Block
						  ListLineInformation[i].no_format = true;
				  }
				  else
				  */
				 // {
					  for (int i = 0; i < ListLineInformation.count(); i++)//hier Format nicht gefunden für die SPS
						  ListLineInformation[i].no_format = true;
				 // }
				  // HalconCpp::HRegion  LineRegion;
				  //Print format center pos into image
				  //FormatImage.GetImageSize(&W, &H);
				  //int num = W / 1024.0;
				  //for (int i = 0; i < num; i++)
				  //{
				  //	  LineRegion.GenRegionLine(0, (Hlong)((i+1)* 1024), H - 1, (Hlong)((i+1)*1024));
				  //	  LineRegion.OverpaintRegion(FormatImage, 190, "fill");
				  //}
				  //QString Path = QString("d://Temp//FormatNotFound.bmp");
				  //FormatImage.WriteImage("bmp", 0, Path.toLatin1().data());

				  //Fehlerbild in die Anzeige
				  HalconCpp::HImage ErrorImage = FormatImage.ZoomImageFactor(GetImageData()->GetDisplayZoomFactorFormatImage(), GetImageData()->GetDisplayZoomFactorFormatImage(), "constant");
				  m_FormatErrorImage.m_Image   = GetImageData()->CopyHalconImageIntoQtImage(ErrorImage, 3);
				  emit SignalShowFormatNotFoundImage(m_FormatErrorImage);//in die Fehlerbildanzeige
				  GetImageData()->ShowInspectionErrorText(tr("Format Not Found"),true);
				  ImageStartPosNewFormat++;
				  GetImageData()->GetMeasureTaskDetectHose()->SetSharedMemoryHeaderHoseImageStartPosNewFormat(ImageStartPosNewFormat);//setze neue Bildstartposition für den nächsten Durchlauf
                  if (!GetImageData()->GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
                  {
                      if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
                          qDebug() << "error format not found top";
                      else
                          qDebug() << "error format not found bot";
                  }
			  }
		  }
    	  //jetzt die Ergebnisse an die SPS bzw erst in die MainLogic Klasse. Hier immer Schlauch gefunden oder evtl. Format nicht gefunden oder Format Außerhalb der Längentoleranz
		  if (GetImageData()->GetMainAppPrintCheck()->GetPLC())
			  GetImageData()->GetMainAppPrintCheck()->GetPLC()->AppendLineInformation(ListLineInformation, GetImageData()->GetCameraIndex());
	  }//end if if(NumberCameraImagesInBuffer>0)
	  m_SharedMemoryFullHose->unlock();
	}
	return rv;
}


void MeasureTaskFormatCheck::AppendMatchSocoreAndAverage(InspectionWindow *pInspectionWindow)
{
	double AverageValue = 0.0;
	m_ListMatchScoresForamtDetection.append(pInspectionWindow->m_Results.m_ModelScore);
	if (m_ListMatchScoresForamtDetection.count() >= 100)
		m_ListMatchScoresForamtDetection.pop_front();

	for (int i = 0; i < m_ListMatchScoresForamtDetection.count(); i++)
		AverageValue = AverageValue + m_ListMatchScoresForamtDetection[i];
	AverageValue = AverageValue / m_ListMatchScoresForamtDetection.count();
	pInspectionWindow->m_Results.m_MeanGrayValue = AverageValue;

}


bool MeasureTaskFormatCheck::CalculateFormatLenght(InspectionWindow *pInspectionWindow,double XPos, int StartPos)
{
	double XPosInHoseCoordinates = XPos + (StartPos * GetImageData()->GetCameraHeightInPixel());
	bool FormatMissing = false;

	if (m_LastXPosInHoseCoordinates > 0.0)
	{
		if (XPosInHoseCoordinates > m_LastXPosInHoseCoordinates)
		{
			double MaxLengthErrorInMM          = 10.0;//Annahme, wenn die Formatlänge um diesen Wert größer ist, dann wird das Format wahrscheinlich nicht gefunden oder die Druckkontrolle wird einen druckfehler erkennen
			int    FormatLenghtDefault         = GetImageData()->GetFormatLenghtInPixel();
			int    CalculatetFormatLenght      = XPosInHoseCoordinates - m_LastXPosInHoseCoordinates;
			double DeltaXInMM                  = abs(FormatLenghtDefault - CalculatetFormatLenght)*GetImageData()->GetPixelSize();
			if(DeltaXInMM < MaxLengthErrorInMM)
			   pInspectionWindow->m_Results.m_FormatLengthInPix = (XPosInHoseCoordinates - m_LastXPosInHoseCoordinates);
			else
				FormatMissing = true;
		}
	}
	m_LastXPosInHoseCoordinates = XPosInHoseCoordinates;
	return FormatMissing;
}


bool MeasureTaskFormatCheck::CheckIsFormatInTolerance(InspectionWindow *pInspectionWindow)
{//laut Lastenheft hier nur die Längentoleranz
	int FormatLenghtDefault = GetImageData()->GetFormatLenghtInPixel();
	//double DeltaXInMM = (pInspectionWindow->m_Results.m_ObjectSizeInX - pInspectionWindow->m_ModelWidthReference)  * GetImageData()->GetPixelSize();
	double DeltaXInMM = abs(FormatLenghtDefault - pInspectionWindow->m_Results.m_FormatLengthInPix)*GetImageData()->GetPixelSize();

	if (pInspectionWindow->m_Results.m_FormatLengthInPix > 0)
	{
		if (fabs(DeltaXInMM) > GetImageData()->GetFormatLenghtTolInMM())
		{
			pInspectionWindow->m_Results.m_FormatLenghtOk  = false;
			pInspectionWindow->m_Results.m_LenghtErrorInMM = DeltaXInMM;
		}
		else
		{
			pInspectionWindow->m_Results.m_FormatLenghtOk = true;
		}
	}
	return pInspectionWindow->m_Results.m_FormatLenghtOk;
}


int MeasureTaskFormatCheck::CheckSharedMemorySizeFormatImage(int NewWidth, int NewHeight, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	unsigned long MaxFormatSizeInBytes;

	if (m_VideoHeaderFormatImage.m_MAXVideoBockSize == 0)
	{//setze shared memory erstmal auf das Maximum(Hier 250mm)
		MaxFormatSizeInBytes = GetImageData()->GetMaxFormatLenghtInPixel()*GetImageData()->GetCameraWidthInPixel();
	}
	else
	{//sollte während des Betriebs das Maximum überschritten, dann wird der Sharedmemory neu angelegt
		MaxFormatSizeInBytes = GetImageData()->GetFormatLenghtInPixel()*GetImageData()->GetCameraWidthInPixel();//Aktuelle Formatgröße
	}
	//Prüfung ob neues Bild in den SharedMemory passt
	if (m_VideoHeaderFormatImage.m_MAXVideoBockSize < MaxFormatSizeInBytes)
	{//sharedmemory neu anlegen damit Formatbild in den Speicher passt
		m_VideoHeaderFormatImage.m_MAXVideoBockSize = MaxFormatSizeInBytes;
		m_VideoHeaderFormatImage.m_ImageWidth       = NewWidth;
		m_VideoHeaderFormatImage.m_ImageHeight      = NewHeight;
		m_VideoHeaderFormatImage.m_ImageBlockSize   = NewWidth * NewHeight;
		GetImageData()->DetachSharedMemoryInPrintchecktask();
		m_SharedMemoryFormatImage->lock();
		if (m_SharedMemoryFormatImage->isAttached())
			m_SharedMemoryFormatImage->detach();
		if (!m_SharedMemoryFormatImage->isAttached())
		{
			rv = m_SharedMemoryFormatImage->CreateNew(m_VideoHeaderFormatImage.m_MAXVideoBockSize + sizeof(VideoHeader), ErrorMsg);//shared memory einmalig anlegen
			if (rv == ERROR_CODE_NO_ERROR)
				memcpy(m_SharedMemoryFormatImage->GetSharedMemoryStartPointer(), &m_VideoHeaderFormatImage, sizeof(VideoHeader));//copy videoHeader initial value
		}
		m_SharedMemoryFormatImage->unlock();
	}
	else
	{
		if (m_VideoHeaderFormatImage.m_ImageWidth != NewWidth || m_VideoHeaderFormatImage.m_ImageHeight != NewHeight)
		{
			m_VideoHeaderFormatImage.m_ImageWidth  = NewWidth;
			m_VideoHeaderFormatImage.m_ImageHeight = NewHeight;
		    m_SharedMemoryFormatImage->lock();
			memcpy(m_SharedMemoryFormatImage->GetSharedMemoryStartPointer(), &m_VideoHeaderFormatImage, sizeof(VideoHeader));
			m_SharedMemoryFormatImage->unlock();
     	}
	}
	return rv;
}


int MeasureTaskFormatCheck::AddFormatImage(HalconCpp::HImage  &FormatImageImage, InspectionWindow *pInspectionWindowFormat, unsigned __int64 ImageTimeStampInMuSec, double HoseMiddlePosition, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	if (GetImageData())
	{
		HalconCpp::HTuple Type, W, H, S;

		try
		{
			GetImagePointer1(FormatImageImage, &S, &Type, &W, &H);
		}
		catch (HalconCpp::HException &exception)
		{//schwerwiegender Fehler
			ErrorMsg = tr("Can Not Put Halcon Image Into Shared Memory. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			return ERROR_CODE_ANY_ERROR;
		}
		unsigned char *pImagePointer = (unsigned char*)(S.L());
		rv = CheckSharedMemorySizeFormatImage(W, H, ErrorMsg);
		if (m_SharedMemoryFormatImage && m_SharedMemoryFormatImage->isAttached())
		{
			unsigned char *pVideoData = NULL;
			int ImageBlockSize = W * H;
			
			m_SharedMemoryFormatImage->lock();
			pVideoData = m_SharedMemoryFormatImage->GetSharedMemoryStartPointer();
			memcpy(&m_VideoHeaderFormatImage, pVideoData, sizeof(VideoHeader));//read video header
			m_VideoHeaderFormatImage.m_InspectionWindowFormatResults   = *pInspectionWindowFormat;
			m_VideoHeaderFormatImage.m_TimeStampFormatImage = ImageTimeStampInMuSec;
			m_VideoHeaderFormatImage.m_HoseMiddlePosition   = HoseMiddlePosition;
			pVideoData = pVideoData + sizeof(VideoHeader);
			memcpy(pVideoData, pImagePointer, ImageBlockSize);
			memcpy(m_SharedMemoryFormatImage->GetSharedMemoryStartPointer(), &m_VideoHeaderFormatImage, sizeof(VideoHeader)); //write video header
			m_SharedMemoryFormatImage->unlock();
			m_SharedMemoryFormatImage->SetEventNewData();//signal an den Thread(MeasureTaskPrintcheck) neues Formatbild im Speicher
		}
	}
	return rv;
}



void MeasureTaskFormatCheck::StartInspection()
{
	start();
}


