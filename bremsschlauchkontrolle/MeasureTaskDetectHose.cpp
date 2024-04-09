#include "MeasureTaskDetectHose.h"
#include "ImageData.h"
#include "GlobalConst.h"
#include "SharedMemoryVideoData.h"
#include "HoseDetector.h"
#include "MatchingShapeBased.h"
#include "CameraSimulation.h"
#include "MainAppPrintCheck.h"
#include "CameraImageAndTimeStamps.h"
#include "MainLogic.h"
#include "PrintLineDetector.h"



MeasureTaskDetectHose::MeasureTaskDetectHose(ImageData *pImageData) : QThread()
, m_ImageData(NULL)//Parent Class
, m_SharedMemoryFullHose(NULL)//Bildspeicher für kompletten Schlauch
, m_TimeoutValueWaitForNextImage(10000)//Timemout Wert wenn die Kamera kein Bild liefert
, m_HoseDetector(NULL)//Messalogorithmus zu bestimmen der Schlauchposition im Kamerabild
, m_TerminateInspection(false)//Wenn auf true dann wird der laufende Thread(Inspection) beendet
, m_EnableGenerateNewReferenceImage(false)//Wenn auf true gesetzt wird ein neues Referenzbild aufgenommen
, m_ResultHoseDetection(STATE_HOSE_UNDEFINED)//Aktueller Sclauchstatus: Schlauchanfang, Schlauchende und Schlauchmitte
, m_ImageCounter(0)//Aktuelle Formatenummer/Bildnummer pro Schlauch
, m_NumberImagesAverageHosePositionResults(6)//Anzahl der Messwerte für die Schlauchposition, die laufend gemittelt werden
, m_FirstCallCalculateFramesPerSecond(true)//Benutzt für die Berechnung der Kameraframerate
, m_CameraFrameCounter(0)//Wird benutzt um die Kameraframerate zu berechnen
, m_EnableWriteFullHose(false)//wenn auf true dann wird der komplette Schlauch abgespeichert
, m_SaveImageCounter(0)//Bildnummer für einen Schlauch der auf der Festplatte gespeichert wird
, m_CalculatetPixelSize(0.0)//Berechnete Pixelgröße
, m_EventFromPLCTubeEndIsReached(false)//Wird von der SPS auf true gesetzt wenn Schlauchende Signal anliegt
, m_EnableGetNewImage(true)//wenn auf false wird der Bildeinzug gesperrt
, m_CounterHoseNotFound(0)//Zählt wie oft hintereinander ein Schlauch nicht gefunden Wurde
{
	QString Msg;
	m_ImageData            = pImageData;//Parent class
	m_HoseDetector         = new HoseDetector(GetImageData());//Messalgorithmus um den die Schlauchposition zu bestimmen
	m_SharedMemoryFullHose = new SharedMemoryVideoData();//Bildspeicher für kompletten Schlauch
	m_SharedMemoryFullHose->SetKeyName(QString("%1%2").arg(SHARED_MEMORY_KEYNAME_FULL_HOSE_IMAGE).arg(GetImageData()->GetCameraIndex() + 1));

	if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
		Msg = tr("Write Full Hose Image Camera Top");
	else
		Msg = tr("Write Full Hose Image Camera Bot");
	m_MessageBox = new BMessageBox(QMessageBox::Warning, Msg, tr("Please Wait..!"));//Infodialog erscheint wenn ein Schlauch gerade gespeichert wird

	connect(this,  &MeasureTaskDetectHose::SignalShowMessageBox, this,  &MeasureTaskDetectHose::SlotShowMessageBox);//Öffnet die MessageBox wenn Schlauch speichern gestartet wird
	connect(this,  &MeasureTaskDetectHose::SignalHideMessageBox, this,  &MeasureTaskDetectHose::SlotHideMessageBox);//Schließt die MessageBox wieder wenn speichern beendet 
	
}


void MeasureTaskDetectHose::LoadSettings()
{
	if (GetHoseDetector())
		GetHoseDetector()->LoadSettings();//Laden der Messparameter
}


MeasureTaskDetectHose::~MeasureTaskDetectHose()
{
	WaitForFinshed();//warte bis thred beendet
	if (GetHoseDetector())
	{
		delete m_HoseDetector;
		m_HoseDetector = NULL;
	}
	if (m_SharedMemoryFullHose)
	{
		delete m_SharedMemoryFullHose;
		m_SharedMemoryFullHose = NULL;
	}
	if (m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec != NULL)
	{
		delete[] m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec;
		m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec = NULL;
	}
}


void MeasureTaskDetectHose::SlotShowMessageBox()
{
	if (m_MessageBox)
		m_MessageBox->show();
}


void MeasureTaskDetectHose::SlotHideMessageBox()
{
	if (m_MessageBox)
		m_MessageBox->hide();
}

//Sharedmemory für einen kompletten Schlauch anlegen wenn er noch nicht angelegt wurde oder bestimmte Parameter haben sich geändert
int MeasureTaskDetectHose::CheckSharedMemoryFullHose(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	if (m_SharedMemoryFullHose)
	{
		if (GetImageData()->GetCameraHeightInPixel() > 0)
		{
			if (m_SharedMemoryHeaderHoseImage.m_ImageHeight != GetImageData()->GetCameraHeightInPixel() || m_SharedMemoryHeaderHoseImage.m_ImageWidth != GetImageData()->GetCameraWidthInPixel())
			{
				int NumberCameraImages = (GetImageData()->GetFullHoseLenghtInPixel() / GetImageData()->GetCameraHeightInPixel()) * 2;
				m_SharedMemoryHeaderHoseImage.m_ImageHeight       = GetImageData()->GetCameraHeightInPixel();
				m_SharedMemoryHeaderHoseImage.m_ImageWidth        = GetImageData()->GetCameraWidthInPixel();
				m_SharedMemoryHeaderHoseImage.m_ImageBlockSize    = m_SharedMemoryHeaderHoseImage.m_ImageHeight * m_SharedMemoryHeaderHoseImage.m_ImageWidth;
				m_SharedMemoryHeaderHoseImage.m_MaxNumberFrames   = NumberCameraImages;//Anzahl Kamerabilder volle Schlauchlänge
				m_SharedMemoryHeaderHoseImage.m_MaxNumberLines    = m_SharedMemoryHeaderHoseImage.m_ImageHeight * m_SharedMemoryHeaderHoseImage.m_MaxNumberFrames;
				m_SharedMemoryHeaderHoseImage.m_MAXVideoBockSize  = m_SharedMemoryHeaderHoseImage.m_MaxNumberLines * GetImageData()->GetCameraWidthInPixel();
				if (m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec != NULL)
				{
					delete[] m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec;
				}
				m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec = new unsigned __int64[NumberCameraImages];//Zeitstempel für jedes Bild
				m_SharedMemoryFullHose->lock();
				if (m_SharedMemoryFullHose->isAttached())
					m_SharedMemoryFullHose->detach();
					rv = m_SharedMemoryFullHose->CreateNew(m_SharedMemoryHeaderHoseImage.m_MAXVideoBockSize, ErrorMsg);//shared memory einmalig anlegen
				m_SharedMemoryFullHose->unlock();
			}
		}
		else
		{
			ErrorMsg = tr("Fatal Error Camera Height Is Zero");
			rv = ERROR_CODE_ANY_ERROR;
		}
	}
	else
	{
		ErrorMsg = tr("No Shared Memory Is Allocatet");
		rv = ERROR_CODE_ANY_ERROR;
		
	}
	return rv;
}

//Neues Bild von der Kamera in den Hauptspeicher für den Schlauch
int MeasureTaskDetectHose::AppendNewImageIntoSharedMemory(CameraImageAndTimeStamps &cameraImageAndTimeStamps,QString &ErrorMsg)
{
	unsigned char *pShareData    = NULL;
	unsigned char *pImagePointer = NULL;
	HalconCpp::HTuple Type, W, H, S;
	int rv = ERROR_CODE_NO_ERROR;
	int ImageBlockSize;



	if (GetImageData()->IsResumeMeasuring())
	{//Messung steht auf an, bzw. auf Produktion
		if (m_StartInspectionIsClicked)//Sicherstellen, dass beim drücken auf Start(Produktion) der Speicher einmal auf Anfang/gelöscht gesetzt wird
		{
			m_ClearBuffer = true;//beim ersten Start Bildspeicher einmal löschen
			m_StartInspectionIsClicked = false;
		}
		else
			m_ClearBuffer = false;
	}
	else
	{
		m_StartInspectionIsClicked = true;//Stop Messung ist gedrückt
		emit SignalMeasuringIsStopped(GetImageData()->GetCameraIndex());
	}
	
	if (m_SharedMemoryFullHose)
	{
		/*rv = CheckSharedMemoryFullHose(ErrorMsg);//Anlegen des Sharedmemory
		if (rv != ERROR_CODE_NO_ERROR)
		{
			emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
			return ERROR_CODE_ANY_ERROR;
		}
		*/
		m_SharedMemoryFullHose->lock();
     	pShareData = m_SharedMemoryFullHose->GetSharedMemoryStartPointer();
		if (pShareData)
		{
			if (m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames >= m_SharedMemoryHeaderHoseImage.m_MaxNumberFrames || m_ClearBuffer)
			{
				ClearSharedMemory();
			}
			try
			{
				GetImagePointer1(cameraImageAndTimeStamps.m_CameraImage, &S, &Type, &W, &H);
			}
			catch (HalconCpp::HException &exception)
			{//schwerwiegender Fehler
				ErrorMsg = tr("Can Not Put Halcon Image Into Shared Memory. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
				m_SharedMemoryFullHose->unlock();
				emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
				return ERROR_CODE_ANY_ERROR;
			}
			ImageBlockSize = m_SharedMemoryHeaderHoseImage.m_ImageHeight*m_SharedMemoryHeaderHoseImage.m_ImageWidth;
			pImagePointer  = (unsigned char*)(S.L());
			pShareData     = m_SharedMemoryFullHose->GetSharedMemoryStartPointer() + (m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames*ImageBlockSize);
			if (pImagePointer && pShareData)
			{
				memcpy(pShareData, pImagePointer, ImageBlockSize);//Bild in den Shared Memory
				m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec[m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames] = cameraImageAndTimeStamps.m_TimeStampInMuSec;//Zeitstempel wann das Bild Aufgenommen wurde
				if (m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames < m_SharedMemoryHeaderHoseImage.m_MaxNumberFrames)
					m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames = m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames + 1;//Bildzähler um eins erhöhen
				m_SharedMemoryHeaderHoseImage.m_ResultsHoseDetection = m_CurrentAverageResultsHoseDetection;//die gemittelten Messdaten des Schlauches speichern
			}
			m_SharedMemoryFullHose->unlock();
			m_SharedMemoryFullHose->SetEventNewData();//Event an die Task FormatCheck neues Formatbild Im Sharedmemory
		}
	}
	return rv;
}

//Liefert Zeitstempel für eine bestimmet Bildzeile
unsigned __int64 MeasureTaskDetectHose::GetImageTimeStampByImageIndex(int index)
{
	if (index > -1 && index < m_SharedMemoryHeaderHoseImage.m_MaxNumberFrames)
		return m_SharedMemoryHeaderHoseImage.m_ImageTimeStampsInMuSec[index];
	else
		return 0;
}

//started die Inspektion im Kamerabild
void MeasureTaskDetectHose::StartInspection()
{
	if(!isRunning())
	   start(QThread::TimeCriticalPriority);
}

//Neues Bild von der Kamera
void MeasureTaskDetectHose::NewIncommingImage(const cv::Mat &image, bool FromCamera)
{//new Image from Camera or from camera simulation(if FromCamera==false)
	if (isRunning() && m_EnableGetNewImage)
	{
		QString ErrorMsg;
		CameraImageAndTimeStamps cameraImageAndTimeStamps;
		cameraImageAndTimeStamps.m_TimeStampInMuSec = GetImageData()->GetMainAppPrintCheck()->GetCurrentTimeStamp();
		if (image.rows == GetImageData()->GetCameraHeightInPixel() && image.cols == GetImageData()->GetCameraWidthInPixel())
		{
			if (cameraImageAndTimeStamps.AddToHalconImage(image.rows, image.cols, image.data, FromCamera, ErrorMsg) == ERROR_CODE_NO_ERROR)
			{
				CalculateFramesPerSecond();
				m_MutexNewImage.lock();
				m_QQueueIncommingImages.enqueue(cameraImageAndTimeStamps);
				m_MutexNewImage.unlock();
				m_WaitConditionNewImage.wakeAll();//Signal an den Thread neues Bild von der Kamera
			}
			else
			{
				emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
			}
		}
		else
		{
			ErrorMsg = tr("False Camera Size. W:%1 H:%2").arg(image.cols).arg(image.rows);
			emit SignalShowMessage(ErrorMsg, QtMsgType::QtDebugMsg);
		}
	}
	else
	{//wird Bildeinzug unterbrochen oder Thread läuft nicht, muss die Liste geleert werden
		m_MutexNewImage.lock();
		while (!m_QQueueIncommingImages.isEmpty())
		        m_QQueueIncommingImages.dequeue();
		m_MutexNewImage.unlock();
	}
}


//berechnung FPS bzw. Kameraintervall
void MeasureTaskDetectHose::CalculateFramesPerSecond()
{
	if (m_FirstCallCalculateFramesPerSecond)
	{
		m_CameraFrameCounter = 0;
		m_TimerCameraIntervall.start();
		m_FirstCallCalculateFramesPerSecond = false;
	}
	else
	{
		m_CameraFrameCounter++;
		double DiffTimeInms = m_TimerCameraIntervall.nsecsElapsed() / 1000000.0;
		if (DiffTimeInms >= 2000.0)
		{//calculate new every two seconds
			DiffTimeInms = DiffTimeInms / 1000.0;
			m_FirstCallCalculateFramesPerSecond = true;
			emit SignalShowCameraFramesPerSecond(m_CameraFrameCounter / DiffTimeInms);
		}
	}
}

//Thread der die eingehenden Bilder von der Kamera verarbeitet. 
void MeasureTaskDetectHose::run()
{
	QMutex Mutex;
	int waitTime = 0;
	int waitTimeWhenCameraTimeout = 5000;

	m_StartInspectionIsClicked=true;
	m_ClearBuffer=false;
	while (!m_TerminateInspection)
	{
		Mutex.lock();
		if (m_QQueueIncommingImages.isEmpty())
		{//wenn kein Bild in der Queue dann warten
			if (!m_WaitConditionNewImage.wait(&Mutex, m_TimeoutValueWaitForNextImage))
			{//kein Bild von der Kamera
				QProcess RestartApplication;
				QString BatFileName = QFileInfo(QCoreApplication::applicationFilePath()).baseName() + ".bat";
				QString DirPath = qApp->applicationDirPath();
				QString BatchFileLocationAndName = DirPath + QString("/") + BatFileName;
				if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
				{
				    emit SignalShowMessage(tr("Timeout No Image From Camera Top"), QtMsgType::QtFatalMsg);
					if (GetImageData()->GetMainAppPrintCheck()->RestartApplicationWhenCameraTimeout()) {
						msleep(waitTimeWhenCameraTimeout);//Wait write error message before quit application
						m_TerminateInspection = true;
					    if (!GetImageData()->GetMainAppPrintCheck()->IsProgramHasAlreadyEnded()) {
							GetImageData()->GetMainAppPrintCheck()->SetProgramHasAlreadyEnded(true);
							QApplication::quit();
							RestartApplication.startDetached(BatchFileLocationAndName, QStringList());
    					}
				    }
				}
				else
				{
					emit SignalShowMessage(tr("Timeout No Image From Camera Bot"), QtMsgType::QtFatalMsg);
					if (GetImageData()->GetMainAppPrintCheck()->RestartApplicationWhenCameraTimeout()) {
						msleep(waitTimeWhenCameraTimeout);//Wait write error message before quit application
						m_TerminateInspection = true;
						if (!GetImageData()->GetMainAppPrintCheck()->IsProgramHasAlreadyEnded()) {
							GetImageData()->GetMainAppPrintCheck()->SetProgramHasAlreadyEnded(true);
							QApplication::quit();
							RestartApplication.startDetached(BatchFileLocationAndName, QStringList());
	    				}
			    	}
				}
		    }
		}
		if (m_TerminateInspection)break;//Abbruch, Programm wird beendet
		Mutex.unlock();

		while (!m_QQueueIncommingImages.isEmpty())
		{
			if (m_MutexNewImage.tryLock())
			{//Camerabild mit Zeitstempel einlesen
				CameraImageAndTimeStamps cameraImageAndTimeStamps = m_QQueueIncommingImages.dequeue();

				waitTime = 0;
				if (m_EventFromPLCTubeEndIsReached)
				{//Schlauchende Signal von der SPS, lösche Bildspeicher(Alles auf Anfang) und erhöhe Schlauchzähler um eins
					IncrementHoseCounterAndClearSharedMemory();
					m_EventFromPLCTubeEndIsReached = false;
					if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
						waitTime = GetImageData()->GetMainAppPrintCheck()->GetWaitTimeAfterTubEndCameraTopInms();
					else
						waitTime = GetImageData()->GetMainAppPrintCheck()->GetWaitTimeAfterTubEndCameraBotInms();
					if (waitTime > 0)
					{
						m_EnableGetNewImage = false;//Bildeinzug sperren
					}
				}
				else
				{
					DedectHoseAndAddImage(cameraImageAndTimeStamps);//Schauchsuche
				}
				m_MutexNewImage.unlock();

				if (!m_EnableGetNewImage)
				{
					msleep(waitTime);
					m_EnableGetNewImage = true;
				}
	   		}
			else
				msleep(0);
		}
	}
	m_WaitConditionLiveViewIsDisable.wakeAll();
}

//sucht den Schlauch im Kamerabild, wenn gefunden, dann wird ein Signal an die Task gesendet die das Format sucht
void MeasureTaskDetectHose::DedectHoseAndAddImage(CameraImageAndTimeStamps &cameraImageAndTimeStamps)
{
	int rv= ERROR_CODE_NO_ERROR;
	QString ErrorMsg;
	
	rv = DetectHose(cameraImageAndTimeStamps,ErrorMsg);//Bestimmung Schlauchposition bzw. ist ein Schlauch im Kamerabild
	if (rv == ERROR_CODE_NO_ERROR)
	{//Messung erfolgreich
		if (m_LiveCameraImageGUI.m_ListInspectionWindowResults.count() == 1)
		{//Messergebnis vorhanden
			if (m_LiveCameraImageGUI.m_ListInspectionWindowResults.at(0).m_Results.m_ModelFound)
			{//Schlauch gefunden
				AppendNewImageIntoSharedMemory(cameraImageAndTimeStamps, ErrorMsg);//wenn schlauch gefunden dann Bild im Sharedmemory speichern
				if (GetImageData()->GetCheckNewReference())
				{//Aufnahme neues Referenzbild und Anzeige wir befinden uns hier im Einrichten, es wird ein neues Referenzbild angefordert
					if (!m_EnableGenerateNewReferenceImage)
					{//Bildspeicher einmal komplett löschen um eine neue Referenz aufzunehmen
						ClearSharedMemory();
						m_EnableGenerateNewReferenceImage = true;
					}
					rv = PutNewReferenceImage(ErrorMsg);//Prüfung sind genügend Einzelbilder aufgenommen
					if (rv == ERROR_CODE_ANY_ERROR || rv == RETURN_CODE_REFERENCE_READY)
					{
						if (rv == ERROR_CODE_ANY_ERROR)
							emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
						GetImageData()->SetCheckNewReference(false);//wieder auf normalen Messmodus umschalten
						m_EnableGenerateNewReferenceImage = false;
					}
		    	}
				emit SignalHoseFound(true);
				m_CounterHoseNotFound = 0;
    		}
			else
			{//hose not found
				m_CounterHoseNotFound++;
				int MaxValue;
				if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
					MaxValue = GetImageData()->GetMainAppPrintCheck()->GetMaxNumberCameraTopImagesHoseNotFound();
				else
					MaxValue = GetImageData()->GetMainAppPrintCheck()->GetMaxNumberCameraBotImagesHoseNotFound(); 
				if (m_CounterHoseNotFound > MaxValue)//hier Prüfung ab wann Schlauch nicht gefunden ausgelöst wird
				{
					emit SignalHoseFound(false);//Anzeige das kein Schlauch im Bild
					m_CounterHoseNotFound = 0;
					ClearSharedMemory();//speicher auf Null alles auf Anfang
					GetImageData()->ResetFormatCounter();//Bildzähler auf Null
					if (!GetImageData()->IsResumeMeasuring() && !GetImageData()->GetCheckNewReference())
						SetDataToPLCHoseNotFound(cameraImageAndTimeStamps.m_TimeStampInMuSec);//Messung ist nicht gestopped und es wird auch kein neues Referenzbild aufgenommen, dann Ergebnis and die SPS
				}
				else
				{
					if (GetImageData()->IsResumeMeasuring() && GetImageData()->GetFormatCounter()>0)
					   AppendNewImageIntoSharedMemory(cameraImageAndTimeStamps, ErrorMsg);//Bild trotzdem uebernehmen aber nur wenn auf Produktion steht und einmal das Format vorab gefunden wurde
				}
				
			}
			if (m_LiveCameraImageGUI.m_ListInspectionWindowResults.count() > 0)
				m_LiveCameraImageGUI.m_ListInspectionWindowResults[0].m_Results.m_ResultHoseDetection = m_ResultHoseDetection;//Ergebnis der Schlauchposition
			emit SignalShowLiveImage(m_LiveCameraImageGUI);//Kamerbild in die Anzeige
		}
	}
	else
	{//Fatal error can not measure
		emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
	}
}

//Automatische Schlauchanfang und Schlauchende Erkennung momentan nicht genutzt
void MeasureTaskDetectHose::SetStateHoseFound()
{
	if (m_ResultHoseDetection == STATE_HOSE_UNDEFINED)
	{
		m_ResultHoseDetection = m_ResultHoseDetection | STATE_HOSE_FOUND;
	}
	else
	{
		if (m_ResultHoseDetection & STATE_HOSE_NOT_FOUND)
		{
			m_ResultHoseDetection = 0;
			m_ResultHoseDetection = m_ResultHoseDetection | STATE_HOSE_FOUND;
			m_ResultHoseDetection = m_ResultHoseDetection | STATE_HOSE_START_POINT_FOUND;
		}
	}
}

//Automatische Schlauchanfang und Schlauchende Erkennung momentan nicht genutzt
void MeasureTaskDetectHose::SetStateHoseNotFound()
{
	if (m_ResultHoseDetection == STATE_HOSE_UNDEFINED)
	{
		m_ResultHoseDetection = m_ResultHoseDetection | STATE_HOSE_NOT_FOUND;
	}
	else
	{
		if (m_ResultHoseDetection & STATE_HOSE_FOUND)
		{
			m_ResultHoseDetection = 0;
			m_ResultHoseDetection = m_ResultHoseDetection | STATE_HOSE_NOT_FOUND;
			m_ResultHoseDetection = m_ResultHoseDetection | STATE_HOSE_END_POINT_FOUND;
		}
	}
}

//Wird durch MainLogic(SPS) gesetzt, wenn Schlauchende erreicht
void MeasureTaskDetectHose::EventFromPLCTubeEndIsReached()
{
	m_EventFromPLCTubeEndIsReached = true;
}

//Wenn Schlauchende erreicht dann BildSpeicher löschen und Formatzähler auf 0 wenn Format nicht gefunden. Alles auf Anfang 
void MeasureTaskDetectHose::IncrementHoseCounterAndClearSharedMemory()
{
	if (m_EnableWriteFullHose)
	{
		if (m_SaveImageCounter == 0)
			m_SaveImageCounter = 1;
		else
		    WriteFullHoseImageOnDisk();//erst speichern wenn ein kompletter Schlauch im Speicher
	}
	ClearSharedMemory();
	GetImageData()->ResetFormatCounter();
}

//wird gesetzt wenn ein kompletter Schlauch in eine Datei gespeichert werden soll
void MeasureTaskDetectHose::SetEnableWriteFullHose(bool set) 
{ 
	m_EnableWriteFullHose = set;
	if (set)
		m_SaveImageCounter=0;
}

//Daten an die SPS Schlauch nicht gefunden
void MeasureTaskDetectHose::SetDataToPLCHoseNotFound(unsigned __int64 TimeStampInMuSec)
{
	QList< ImageLineInformation> ListLineInformation;

	GetImageData()->GenerateImageTimeStampList(ListLineInformation, GetImageData()->GetCameraHeightInPixel(), TimeStampInMuSec);
	for (int i = 0; i < ListLineInformation.count(); i++)
		ListLineInformation[i].tube_found = false;
	if (GetImageData()->GetMainAppPrintCheck()->GetPLC())
		GetImageData()->GetMainAppPrintCheck()->GetPLC()->AppendLineInformation(ListLineInformation, GetImageData()->GetCameraIndex());
}

//Einmal kompletten Schlauch speichern mit zusätzlichen Parameter wie z.B. die PixelSize
void MeasureTaskDetectHose::WriteFullHoseImageOnDisk()
{
	unsigned long NumberLines, y;
	QString FileLocation,Error;
	
	m_SharedMemoryFullHose->lock();
	unsigned char *pShareData = m_SharedMemoryFullHose->GetSharedMemoryStartPointer();
	if (pShareData)
	{ 
		    emit SignalShowMessageBox();
		    if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
			    FileLocation = GetImageData()->GetMainAppPrintCheck()->GetVideoFileLocationCameraTop() + QString("/Video%1.%2").arg(m_SaveImageCounter).arg(VIDEO_FILE_EXTENSION);
		    else
			    FileLocation = GetImageData()->GetMainAppPrintCheck()->GetVideoFileLocationCameraBot() + QString("/Video%1.%2").arg(m_SaveImageCounter).arg(VIDEO_FILE_EXTENSION);
		    m_SaveImageCounter++;
			if (m_SaveImageCounter > 5)
				m_SaveImageCounter = 0;//Maximal nur n-Dateien Abspeichern, die älteste Datei wird dann überschrieben
		    NumberLines = m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames*m_SharedMemoryHeaderHoseImage.m_ImageHeight;
			
			QImage *QtImage = new QImage(m_SharedMemoryHeaderHoseImage.m_ImageWidth, NumberLines, QImage::Format_Indexed8);
			
			pShareData = m_SharedMemoryFullHose->GetSharedMemoryStartPointer();
			for (y = 0; y < NumberLines; y++)
			{
			   memcpy(QtImage->scanLine(y), pShareData, m_SharedMemoryHeaderHoseImage.m_ImageWidth);
			   pShareData = pShareData + m_SharedMemoryHeaderHoseImage.m_ImageWidth;
			}
			QImageWriter writer(FileLocation, "");
			
			writer.setQuality(100);
			if (writer.supportsOption(QImageIOHandler::Description))
			{//Informationen für den Offline-Modus, Parameter die bei der Kamerasimulation gesetzt werden. Wichtig da die Parameter an der Anlage mit der am Offline-PC übereinstimmen  
				writer.setText("SubImageWidth", QString("%1").arg(m_SharedMemoryHeaderHoseImage.m_ImageWidth));
				writer.setText("SubImageHeight", QString("%1").arg(m_SharedMemoryHeaderHoseImage.m_ImageHeight));
				writer.setText("PixelSize", QString("%1").arg(GetImageData()->GetPixelSize(), 0, 'f', 7));
				writer.setText("TopCameraFirst", QString("%1").arg(GetImageData()->GetTopCameraIsFirst()));
			}
			writer.write(*QtImage);
			emit SignalHideMessageBox();
	}
	m_SharedMemoryFullHose->unlock();
}

//Erzeuge Referenzbild aus den Einzelbildern von der Kamera
int MeasureTaskDetectHose::PutNewReferenceImage(QString &ErrorMsg)
{
	int CurrentFrameImageHeight,rv = ERROR_CODE_NO_ERROR;
	int NumberCameraImagePerReference;
	unsigned char *pShareData = NULL;
	bool ReadDirectionLeftToRight=true;
	double FormatLenghtInRefViewInPixel = GetImageData()->GetFormatLenghtInPixel() * GetImageData()->GetMainAppPrintCheck()->GetNumberFormatsInReferenceImageView();

	NumberCameraImagePerReference = static_cast<int>(FormatLenghtInRefViewInPixel / m_SharedMemoryHeaderHoseImage.m_ImageHeight);
	if (m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames >= NumberCameraImagePerReference)
	{//genug aufnahemen für ein neues Referenzbild
		if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
			ReadDirectionLeftToRight = !(GetImageData()->GetTopCameraIsFirst());
		else
		    ReadDirectionLeftToRight = (GetImageData()->GetTopCameraIsFirst());
		CurrentFrameImageHeight      = m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames  * m_SharedMemoryHeaderHoseImage.m_ImageHeight;
		m_SharedMemoryFullHose->lock();
	    pShareData = m_SharedMemoryFullHose->GetSharedMemoryStartPointer();
		if (pShareData)
		{
			try
			{
				m_ReferenceImage.GenImage1("byte", static_cast<Hlong>(m_SharedMemoryHeaderHoseImage.m_ImageWidth), static_cast<Hlong>(CurrentFrameImageHeight), (void*)(pShareData));
				m_ReferenceImage = m_ReferenceImage.RotateImage(90.0, "bilinear");
				if (!ReadDirectionLeftToRight)
					m_ReferenceImage = m_ReferenceImage.MirrorImage("column");

				int ScaledWidth = m_SharedMemoryHeaderHoseImage.m_ImageWidth;
				int ScaledHeight = CurrentFrameImageHeight;
				if (GetImageData())
					GetImageData()->GetDisplayZoomedSizeFormatImage(ScaledWidth, ScaledHeight);
				m_ReferenceImageGUI.m_Image = QImage(pShareData, m_SharedMemoryHeaderHoseImage.m_ImageWidth, CurrentFrameImageHeight, QImage::Format_Grayscale8).scaled(ScaledWidth, ScaledHeight);
				m_ReferenceImageGUI.m_Image = m_ReferenceImageGUI.m_Image.transformed(QMatrix().rotate(270));
				if (!ReadDirectionLeftToRight)
					m_ReferenceImageGUI.m_Image = m_ReferenceImageGUI.m_Image.mirrored(true, false);
				
				emit SignalShowNewReferenceImage(m_ReferenceImageGUI);
				rv=RETURN_CODE_REFERENCE_READY;
			}
			catch (HalconCpp::HException &exception)
			{//schwerwiegender Fehler
				ErrorMsg = tr("Error Read Reference Image. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
				rv= ERROR_CODE_ANY_ERROR;
    		}
		}
		m_SharedMemoryFullHose->unlock();
	}
	return rv;
}

//Kamerabildspeicher wird auf 0 gesetzt, dies sind die Startbedingungen für einen Neuen Schlauch
void MeasureTaskDetectHose::ClearSharedMemory()
{
	m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames = 0;
	m_SharedMemoryHeaderHoseImage.m_StartPosNewFormat   = 0;
	if(GetImageData())
	   GetImageData()->ResetFormatCheckData();
}

//Hier wird der Startpunkt gesetzt wann ein neues Format beginnt. Wird in der Task FormatCheck ermittelt
void MeasureTaskDetectHose::SetSharedMemoryHeaderHoseImageStartPosNewFormat(int set, int FormatEndPos)
{
	m_SharedMemoryHeaderHoseImage.m_StartPosNewFormat = set;
	m_SharedMemoryHeaderHoseImage.m_FormatEndXpos     = FormatEndPos;//momentan nicht genutzt
}

//Referenzbild von der Festplatte laden
void MeasureTaskDetectHose::LoadAndShowReferenceImageFromDisk()
{
	QString ReferenceLocation = GetImageData()->GetReferenceLocation() + QString("/") + REFERENCE_IMAGE_FILE_NAME;
	HalconCpp::HTuple Type, W, H, S, R, G, B;
	unsigned char *pImagePointer = NULL;
	QString ErrorMsg;

	try
	{
		m_ReferenceImage.ReadImage(ReferenceLocation.toLatin1().data());
		GetImagePointer1(m_ReferenceImage, &S, &Type, &W, &H);
	}
	catch (HalconCpp::HException &exception)
	{//schwerwiegender Fehler
		ErrorMsg = tr("Can Not Read Reference Image. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
		return ;
	}
	pImagePointer = (unsigned char*)(S.L());
	if (pImagePointer)
	{
		int ScaledWidth  = (int)W;
		int ScaledHeight = (int)H;
		if (GetImageData())
			GetImageData()->GetDisplayZoomedSizeFormatImage(ScaledWidth, ScaledHeight);
		m_ReferenceImageGUI.m_Image = QImage(pImagePointer, (int)W, (int)H, QImage::Format_Grayscale8).scaled(ScaledWidth, ScaledHeight);
		emit SignalShowNewReferenceImage(m_ReferenceImageGUI);
	}
}

//die Größe des Referenzbildes
void MeasureTaskDetectHose::GetReferenceSize(int &w, int &h)
{
	Hlong W, H;
	m_ReferenceImage.GetImageSize(&W, &H);
	w = (int)W;
	h = (int)H;
}

//Aus dem Referenzbild werden für die unterschiedliche Prüfaufgaben entsprechende Referenzdaten generiert
void MeasureTaskDetectHose::GenerateReferenceData()
{
	if (m_ReferenceImage.IsInitialized())
	{
		int rv = ERROR_CODE_NO_ERROR;
		bool AnyError = false;
		QString ErrorMsg;
			
		if (GetImageData() && GetImageData()->GetMatchingShapeBased())
		{
			int num=GetImageData()->GetNumberInspectionWindows();
			for (int i = 0; i < GetImageData()->GetNumberInspectionWindows(); i++)
			{
				InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(i);
				if (pInspectionWindow)
				{
					if (!pInspectionWindow->m_CheckOnlyHorizontalLines)
					{
						if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
						{
							rv = GetImageData()->GetMatchingFormatWindow()->GenerateModelReferenceData(m_ReferenceImage, pInspectionWindow, ErrorMsg);
							if (rv != ERROR_CODE_NO_ERROR)
							{
								emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
								AnyError = true;
							}
						}
						else
						{
							rv = GetImageData()->GetMatchingShapeBased()->GenerateModelReferenceData(m_ReferenceImage, pInspectionWindow, ErrorMsg);
							if (rv != ERROR_CODE_NO_ERROR)
							{
								emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
								AnyError = true;
							}
						}
					}
					else
					{
						rv = GetImageData()->GetPrintLineDetector()->GenerateModelReferenceData(m_ReferenceImage, pInspectionWindow, ErrorMsg);
						if (rv != ERROR_CODE_NO_ERROR)
						{
							emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
							AnyError = true;
						}
					}
    			}
			}
			if (!AnyError)
			{
				GetImageData()->WriteProductData();
				QString ReferenceLocation = GetImageData()->GetReferenceLocation() + QString("/") + REFERENCE_IMAGE_FILE_NAME;
				m_ReferenceImage.WriteImage("bmp", 0, ReferenceLocation.toLatin1().data());
			}
			emit SignalNewReferenceDataGenerate(GetImageData()->GetCameraIndex(), ErrorMsg);
		}
	}
}

//Wartet solange bis Thread beendet
void MeasureTaskDetectHose::WaitForFinshed()
{
	if (isRunning())
	{//thread läuft noch
		m_WaitConditionNewImage.wakeAll();
		m_TerminateInspection = true;
		m_WaitLiveImageViewIsDisable.lock();
		m_WaitConditionLiveViewIsDisable.wait(&m_WaitLiveImageViewIsDisable, 5000);//warte bis livebildanzeige beendet
		m_WaitLiveImageViewIsDisable.unlock();
	}
}

//Suche nach dem Schlauch im Kamerbild
int MeasureTaskDetectHose::DetectHose(CameraImageAndTimeStamps &cameraImageAndTimeStamps,QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;

	if (GetHoseDetector() && GetImageData())
	{
		QElapsedTimer timer;
		InspectionWindow inspectWindow;
		InspectionWindow *pInspectionWindowHoseDetection = GetImageData()->GetInspectionWindowHoseDetection();
		int ScaledWidth  = GetImageData()->GetCameraWidthInPixel();
		int ScaledHeight = GetImageData()->GetCameraHeightInPixel();//Bild in der Anzeige skalieren
		HalconCpp::HTuple Type, W, H, S;
		int HalconImageWidth  = 1024;
		int HalconImageHeight = 1024;

		
		timer.start();
		try
		{
			GetImagePointer1(cameraImageAndTimeStamps.m_CameraImage, &S, &Type, &W, &H);

			HalconImageWidth  = (int)W;
		    HalconImageHeight = (int)H;
			ScaledWidth       = HalconImageWidth;
			ScaledHeight      = HalconImageHeight;
		}
		catch (HalconCpp::HException &exception)
		{
			ErrorMsg = tr("Error Get Halcon Image Pointer In DetectHose.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			rv = ERROR_CODE_ANY_ERROR;
		}
		if (rv == ERROR_CODE_NO_ERROR && pInspectionWindowHoseDetection)
		{
			m_LiveCameraImageGUI.m_ListInspectionWindowResults.clear();//für Livebildanzeige
			GetImageData()->GetDisplayZoomedSizeCameraImage(ScaledWidth, ScaledHeight);//Größe für die Bildanzeige

			if (pInspectionWindowHoseDetection->m_ReferenceRect.height() == 0 || pInspectionWindowHoseDetection->m_ReferenceRect.width() == 0)
			{
				pInspectionWindowHoseDetection->m_ReferenceRect.setHeight(GetImageData()->GetCameraHeightInPixel());
				pInspectionWindowHoseDetection->m_ReferenceRect.setWidth(GetImageData()->GetCameraWidthInPixel());
			}
			pInspectionWindowHoseDetection->m_ModelWidthReference = GetImageData()->GetHoseDiameterInPixel();
			pInspectionWindowHoseDetection->m_ModelHeightReference = pInspectionWindowHoseDetection->m_ReferenceRect.height();
			pInspectionWindowHoseDetection->ClearResults();

			rv = GetHoseDetector()->StartDetection(cameraImageAndTimeStamps.m_CameraImage, ErrorMsg);
			unsigned char *pImagePointer = (unsigned char*)(S.L());
			if (pImagePointer != NULL && HalconImageWidth > 0 && HalconImageHeight > 0)
			{
				bool ReadDirectionLeftToRight;
				if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
					ReadDirectionLeftToRight = !(GetImageData()->GetTopCameraIsFirst());
				else
					ReadDirectionLeftToRight = (GetImageData()->GetTopCameraIsFirst());
				//Kamerabild um 90 Grad drehen
				m_LiveCameraImageGUI.m_Image = QImage(pImagePointer, HalconImageWidth, HalconImageHeight, QImage::Format_Grayscale8).scaled(ScaledWidth, ScaledHeight).transformed(QMatrix().rotate(-90));//Bild nach QImage für die Anzeigen

				if(!ReadDirectionLeftToRight)
				{
				  m_LiveCameraImageGUI.m_Image = m_LiveCameraImageGUI.m_Image.mirrored(true, false);
				}
				
				//m_LiveCameraImageGUI.m_Image = QImage(pImagePointer, HalconImageWidth, HalconImageHeight, QImage::Format_Grayscale8).scaled(ScaledWidth, ScaledHeight);//Bild nach QImage für die Anzeigen

				
				pInspectionWindowHoseDetection->m_Results.m_InspectionTimeInms = timer.nsecsElapsed() / 1000000.0;//Messzeit um die Position des Schlauches zu bestimmen
				if (pInspectionWindowHoseDetection->m_Results.m_ModelFound)
					AppendAndAverageNewResult(pInspectionWindowHoseDetection->m_Results);//Hier in der Klasse die Ergebnisse speichen , um später den Mittelwert für ein Format zu bilden
				//Kopie Daten für die Anzeige, Ergebnisse in das Bildzeichnen
				inspectWindow = *pInspectionWindowHoseDetection;
				m_LiveCameraImageGUI.m_ListInspectionWindowResults.append(inspectWindow);
			}
			else
			{
				if (pImagePointer == NULL)
				   ErrorMsg = tr("Error Get Image Pointer In DetectHose.  PointerZero  W: %1  H: %2").arg(GetImageData()->GetCameraWidthInPixel()).arg(GetImageData()->GetCameraHeightInPixel());
				else
					ErrorMsg = tr("Error Get Image Pointer In DetectHose.  PointerOk  W: %1  H: %2").arg(GetImageData()->GetCameraWidthInPixel()).arg(GetImageData()->GetCameraHeightInPixel());
				rv = ERROR_CODE_ANY_ERROR;
			}
		}
	}
	return rv;
}

//Die Messwerte für die Schlauchposition und Schlauchdurchmesser werden laufen gemittelt
void MeasureTaskDetectHose::AppendAndAverageNewResult(MeasurementResult &Results)
{
	double CurrentPixelSize    = GetImageData()->GetPixelSize();
	double ProductDiameterInMM = GetImageData()->GetHoseDiameterInMM();

	m_ListMeasurementResultsHoseDetection.append(Results);
	if (m_ListMeasurementResultsHoseDetection.count() >= m_NumberImagesAverageHosePositionResults)
		m_ListMeasurementResultsHoseDetection.pop_front();
	m_CurrentAverageResultsHoseDetection.ClearResults();
	for (int i = 0; i < m_ListMeasurementResultsHoseDetection.count(); i++)
	{
		m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX        = m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX        + m_ListMeasurementResultsHoseDetection.at(i).m_ObjectSizeInX;
		m_CurrentAverageResultsHoseDetection.m_ObjectSizeInY        = m_CurrentAverageResultsHoseDetection.m_ObjectSizeInY        + m_ListMeasurementResultsHoseDetection.at(i).m_ObjectSizeInY;
		m_CurrentAverageResultsHoseDetection.m_ResultXPos           = m_CurrentAverageResultsHoseDetection.m_ResultXPos           + m_ListMeasurementResultsHoseDetection.at(i).m_ResultXPos;
		m_CurrentAverageResultsHoseDetection.m_ResultYPos           = m_CurrentAverageResultsHoseDetection.m_ResultYPos           + m_ListMeasurementResultsHoseDetection.at(i).m_ResultYPos;
		m_CurrentAverageResultsHoseDetection.m_ModelScore          = m_CurrentAverageResultsHoseDetection.m_ModelScore            + m_ListMeasurementResultsHoseDetection.at(i).m_ModelScore;
		m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInX = m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInX + m_ListMeasurementResultsHoseDetection.at(i).m_ResultScaleFactorInX;
		m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInY = m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInY + m_ListMeasurementResultsHoseDetection.at(i).m_ResultScaleFactorInY;
	}
	m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX        = m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX        / m_ListMeasurementResultsHoseDetection.count();
	m_CurrentAverageResultsHoseDetection.m_ObjectSizeInY        = m_CurrentAverageResultsHoseDetection.m_ObjectSizeInY        / m_ListMeasurementResultsHoseDetection.count();
	m_CurrentAverageResultsHoseDetection.m_ResultXPos           = m_CurrentAverageResultsHoseDetection.m_ResultXPos           / m_ListMeasurementResultsHoseDetection.count();
	m_CurrentAverageResultsHoseDetection.m_ResultYPos           = m_CurrentAverageResultsHoseDetection.m_ResultYPos           / m_ListMeasurementResultsHoseDetection.count();
	m_CurrentAverageResultsHoseDetection.m_ModelScore          = m_CurrentAverageResultsHoseDetection.m_ModelScore            / m_ListMeasurementResultsHoseDetection.count();
	m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInX = m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInX / m_ListMeasurementResultsHoseDetection.count();
	m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInY = m_CurrentAverageResultsHoseDetection.m_ResultScaleFactorInY / m_ListMeasurementResultsHoseDetection.count();

	if (m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX > 0.0)
		m_CalculatetPixelSize = ProductDiameterInMM / m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX;
	emit SignalShowCalculatetPixelSize(m_CalculatetPixelSize, m_CurrentAverageResultsHoseDetection.m_ObjectSizeInX * CurrentPixelSize,GetImageData()->GetCameraIndex());
}

