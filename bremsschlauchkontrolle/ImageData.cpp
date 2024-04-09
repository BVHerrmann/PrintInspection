#include "ImageData.h"
#include "GlobalConst.h"
#include "LiveImageView.h"
#include "MainAppPrintCheck.h"
#include "MeasureTaskDetectHose.h"
#include "MeasureTaskPrintCheck.h"
#include "MeasureTaskFormatCheck.h"
#include "InspectionWindow.h"
#include "ProductData.h"
#include "ErrorImageView.h"
#include "MainGUIPrintCheck.h"
#include "CameraImageView.h"
#include "TableWidgetSubFormatResults.h"
#include "MatchingShapeBased.h"
#include "QDesktopWidget"
#include "WidgetDetailResults.h"
#include "CameraSimulation.h"
#include "PrintLineDetector.h"
#include "qscreen.h"


ImageData::ImageData(MainAppPrintCheck *pParent, int CameraIndex) : QThread()
, m_MeasureTaskDetectHose(NULL)
, m_MeasureTaskPrintCheck(NULL)
, m_MeasureTaskFormatCheck(NULL)
, m_MainAppPrintCheck(NULL)//Parent
, m_FormatImageView(NULL)//Livebild Anzeige
, m_ErrorImageView(NULL)//Ergebnisbild /Fehlerbild
, m_ReferenceImageView(NULL)
, m_CameraImageView(NULL)
, m_CameraIndex(0)//Zu welcher Kamera gehört diese Instantz mögliche Werte sind CAMERA_ID_TOP = 1 oder CAMERA_ID_BOT = 2
, m_ResumeMeasuring(false)//Beim Start Wird nicht gemessen die SPS startet den Messvorgang
, m_CheckNewReference(false)
, m_CameraWidthInPixel(1024)
, m_CameraHeightInPixel(1024)
, m_TableWidgetSubFormatResults(NULL)
, m_MatchingShapeBased(NULL)
, m_MatchingFormatWindow(NULL)
, m_WidgetDetailResults(NULL)
, m_CameraSimulation(NULL)
, m_FormatCounter(0)
, m_PrintLineDetector(NULL)
, m_TerminateSaveResults(false)
{
	    m_MainAppPrintCheck                                  = pParent;
		m_CameraIndex                                        = CameraIndex;
		m_PrintLineDetector                                  = new PrintLineDetector(this, MEASURE_TOOL_NAME_LINE_CHECK);//Messwerkzeug für Zeilenausfall
		m_MatchingFormatWindow                               = new MatchingShapeBased(this, MEASURE_TOOL_NAME_FORMAT_DETECTION);//Messwerkzeug um das Format zu suchen
		m_MatchingShapeBased                                 = new MatchingShapeBased(this, MEASURE_TOOL_NAME_PRINT_CHECK);//Messwerkzeug für die eigentliche Druckkontrolle
		m_MeasureTaskDetectHose                              = new MeasureTaskDetectHose(this);//thread für Messwerkzeug PrintlineDetector
		m_MeasureTaskPrintCheck                              = new MeasureTaskPrintCheck(this);//thread für Messwerkzeug Formatdetection
		m_MeasureTaskFormatCheck                             = new MeasureTaskFormatCheck(this);//thread für Messwerkzeug PrintCheck
    	m_FormatImageView                                    = new LiveImageView(this,true);//Formatbild Anzeige
		m_ErrorImageView                                     = new ErrorImageView(this);//fehlerbild Anzeige
		m_ReferenceImageView                                 = new LiveImageView(this,false);//Referenzbild Anzeige
		m_CameraImageView                                    = new CameraImageView(this);//Livebild von der Kamera
		m_WidgetDetailResults                                = new WidgetDetailResults(GetMainAppPrintCheck()->GetMainGUIPrintCheck());//Anzeigedialog für die Messergebnisse
		m_CameraSimulation                                   = new CameraSimulation(this);//Möglichkeit die Kamera zu Simulieren, liest einen kompletten aufgenommen Schlauch von der Festplatte(Offline Modus)

		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalShowLiveImage,                m_CameraImageView,    &CameraImageView::SlotShowCameraImage);//Kamaerabild
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalShowMessage,                  m_MainAppPrintCheck,  &MainAppPrintCheck::SlotAddNewMessage);//Fehlermeldung/Schwerwiegender Fehler
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalShowNewReferenceImage,        m_ReferenceImageView, &LiveImageView::SlotShowLiveImage);//Anzeige Referenzbild
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalNewReferenceDataGenerate,     m_MainAppPrintCheck,  &MainAppPrintCheck::SlotNewReferenceDataGeneratet);
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalShowCameraFramesPerSecond,    this,                 &ImageData::SlotShowCameraFramesPerSecond);
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalHoseFound,                    this,                 &ImageData::SlotHoseFound);
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalShowCalculatetPixelSize,      m_MainAppPrintCheck,  &MainAppPrintCheck::SlotShowCalculatetPixelSize);
		connect(m_MeasureTaskDetectHose,  &MeasureTaskDetectHose::SignalMeasuringIsStopped,           m_MainAppPrintCheck,  &MainAppPrintCheck::SlotMeasuringIsStopped);


		connect(m_MeasureTaskPrintCheck,  &MeasureTaskPrintCheck::SignalShowLiveImage,                m_FormatImageView,    &LiveImageView::SlotShowLiveImage);//Formatbild
		connect(m_MeasureTaskPrintCheck,  &MeasureTaskPrintCheck::SignalShowErrorImage,               m_ErrorImageView,     &ErrorImageView::SlotShowErrorImage);//Formatfehlerbild
		connect(m_MeasureTaskPrintCheck,  &MeasureTaskPrintCheck::SignalShowMessage,                  m_MainAppPrintCheck,  &MainAppPrintCheck::SlotAddNewMessage);//Fehlermeldung/Schwerwiegender Fehler

		connect(m_MeasureTaskFormatCheck, &MeasureTaskFormatCheck::SignalShowMessage,                 m_MainAppPrintCheck,  &MainAppPrintCheck::SlotAddNewMessage);//Fehlermeldung/Schwerwiegender Fehler
		connect(m_MeasureTaskFormatCheck, &MeasureTaskFormatCheck::SignalShowFormatNotFoundImage,     m_ErrorImageView,     &ErrorImageView::SlotShowErrorImage);//Formatfehlerbild

		connect(this,                     &ImageData::SignalShowInspectionTime,                       this,                 &ImageData::SlotShowInspectionTime);
		connect(m_CameraSimulation,       &CameraSimulation::SignalSetCameraStatus,                   this,                 &ImageData::SlotSetCameraStatus);
		connect(m_CameraSimulation,       &CameraSimulation::SignalSetSliderValues,                   m_MainAppPrintCheck,  &MainAppPrintCheck::SlotSetSliderValues);
		connect(m_CameraSimulation,       &CameraSimulation::SignalShowMessage,                       m_MainAppPrintCheck,  &MainAppPrintCheck::SlotAddNewMessage);//Fehlermeldung/Schwerwiegender Fehler
		//Tabelle für die Messergebnisse
		m_HeaderListInspectionResults.append(tr("Date"));
		m_HeaderListInspectionResults.append(tr("Time"));
		m_HeaderListInspectionResults.append(tr("BlockID"));
		m_HeaderListInspectionResults.append(tr("InspectTime"));
		m_HeaderListInspectionResults.append(tr("Name"));
		m_HeaderListInspectionResults.append(tr("Found"));
		m_HeaderListInspectionResults.append(tr("Print"));
		m_HeaderListInspectionResults.append(tr("BlockWidth"));
		m_HeaderListInspectionResults.append(tr("BlockHeight"));
		m_HeaderListInspectionResults.append(tr("Width"));
		m_HeaderListInspectionResults.append(tr("Height"));
		m_HeaderListInspectionResults.append(tr("Xpos"));
		m_HeaderListInspectionResults.append(tr("Ypos"));
		m_HeaderListInspectionResults.append(tr("ScaleX"));
		m_HeaderListInspectionResults.append(tr("ScaleY"));
		m_HeaderListInspectionResults.append(tr("MatchScore"));
		m_HeaderListInspectionResults.append(tr("DefectScore"));
		m_HeaderListInspectionResults.append(tr("MiddleDrift"));

		TableWidgetSubFormatResults *pTableWidgetSubFormatResults = new TableWidgetSubFormatResults(this);
		pTableWidgetSubFormatResults->UpdateResults();
		if(GetWidgetDetailResults())
		   GetWidgetDetailResults()->InsertNewTab(0, (QWidget*)(pTableWidgetSubFormatResults),QString(""));
		
		m_Colors.resize(256);
		for (int i = 0; i < m_Colors.size(); i++)
			m_Colors[i] = 0xff000000 | i << 16 | i << 8 | i;
		start();//starte thread zum speichern der Bilder und Messergebnisse
}


ImageData::~ImageData()
{
	WaitForFinshed();
	if (GetCameraSimulation())
	{
		delete m_CameraSimulation;
		m_CameraSimulation = NULL;
	}
	if (m_MatchingShapeBased)
	{
		delete m_MatchingShapeBased;
		m_MatchingShapeBased = NULL;
	}
	if (m_MatchingFormatWindow)
	{
		delete m_MatchingFormatWindow;
		m_MatchingFormatWindow = NULL;
	}
	if (m_PrintLineDetector)
	{
		delete m_PrintLineDetector;
		m_PrintLineDetector = NULL;
	}
}

//Einmalig Speicher für den Kompletten Schlauch anlegen. Wird Aufgerufen wenn die Hauptinstanz erstellwird
void ImageData::CreateSharedmemoryFullHose()
{
	QString ErrorMsg;
	int rv = ERROR_CODE_NO_ERROR;
	if (m_MeasureTaskDetectHose)
		rv=m_MeasureTaskDetectHose->CheckSharedMemoryFullHose(ErrorMsg);//Shared Memory anlegen
	if (rv != ERROR_CODE_NO_ERROR)
	{
		emit SignalShowMessage(ErrorMsg, QtMsgType::QtFatalMsg);
	}
}


void ImageData::SlotHoseFound(bool found)
{
	if (GetFormatImageView())
	{
		if(!found)
		  GetFormatImageView()->ShowHoseNotFoundText();
		else
		  GetFormatImageView()->ClearTextHoseNotFound();
	}
}

//Liefert die List der Messergebnisse pro Messfenster
void ImageData::GetListMeasurementResults(InspectionWindow *pInspectionWindow, QStringList &List)
{
	List.clear();
	if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID != INSPECTION_ID_HOSE_WINDOW)
	{
		double PixelSize = GetPixelSize();
		List.append(QString("%1").arg(pInspectionWindow->m_Results.m_Date));
		List.append(QString("%1").arg(pInspectionWindow->m_Results.m_Time));
		List.append(QString("%1").arg(pInspectionWindow->m_InspectionWindowID));
		List.append(QString("%1ms").arg(pInspectionWindow->m_Results.m_InspectionTimeInms, 0, 'f', 1));
		List.append(QString("%1").arg(pInspectionWindow->m_ModelName));
		//result model found
		if (pInspectionWindow->m_Results.m_ModelFound)
			List.append(QString("Ok"));
		else
			List.append(QString("NotOk"));
		//result print Ok
		if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
		{
			List.append(QString("-"));
		}
		else
		{
			if (pInspectionWindow->m_CheckOnlyHorizontalLines)
			{
				if (pInspectionWindow->m_Results.m_LineCheckOk)
					List.append(QString("Ok"));
				else
					List.append(QString("NotOk"));
			}
			else
			{
				if (pInspectionWindow->m_Results.m_PrintOk)
					List.append((QString("Ok")));
				else
					List.append((QString("NotOk")));
			}
		}
		//result print Ok
		if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
		{
			if (pInspectionWindow->m_Results.m_FormatLenghtOk)
				List.append(QString("Ok"));
			else
				List.append(QString("NotOk"));
			List.append(QString("-"));
		}
		else
		{
			if (pInspectionWindow->m_CheckOnlyHorizontalLines)
			{
				List.append(QString("-"));
				List.append(QString("-"));
			}
			else
			{
				if (pInspectionWindow->m_Results.m_BlockWidthOk)
					List.append(QString("Ok"));
				else
					List.append(QString("NotOk"));

				if (pInspectionWindow->m_Results.m_BlockHeightOk)
					List.append(QString("Ok"));
				else
					List.append(QString("NotOk"));
			}
		}
		if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
	       List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_FormatLengthInPix *PixelSize, 0, 'f', 1));
		else
		   List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_ObjectSizeInX *PixelSize, 0, 'f', 1));
		List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_ObjectSizeInY *PixelSize, 0, 'f', 1));
		List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_ResultXPos*PixelSize, 0, 'f', 1));
		List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_ResultYPos*PixelSize, 0, 'f', 1));
		List.append(QString("%1").arg(pInspectionWindow->m_Results.m_ResultScaleFactorInX, 0, 'f', 1));
		List.append(QString("%1").arg(pInspectionWindow->m_Results.m_ResultScaleFactorInY, 0, 'f', 1));
		if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
		  List.append(QString("%1%(%2)").arg(pInspectionWindow->m_Results.m_ModelScore, 0, 'f', 1).arg(pInspectionWindow->m_Results.m_MeanGrayValue, 0, 'f', 1));
		else
		  List.append(QString("%1%").arg(pInspectionWindow->m_Results.m_ModelScore, 0, 'f', 1));
		//result defect score and Y-Offset
		if (pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW || pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_HOSE_WINDOW)
		{
			if(pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW && !(pInspectionWindow->m_Results.m_FormatLenghtOk))
			   List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_LenghtErrorInMM, 0, 'f', 2));
			else
			   List.append(QString("-"));
			List.append(QString("-"));
		}
		else
		{
			if (pInspectionWindow->m_CheckOnlyHorizontalLines)
			{
				List.append(QString("%1%(%2)").arg(pInspectionWindow->m_Results.m_DefectScore, 0, 'f', 1).arg(pInspectionWindow->m_Results.m_MeanGrayValue, 0, 'f', 0));
				List.append(QString("-"));
			}
			else
			{
				List.append(QString("%1%").arg(pInspectionWindow->m_Results.m_DefectScore, 0, 'f', 1));
				List.append(QString("%1mm").arg(pInspectionWindow->m_Results.m_OffsetY*PixelSize, 0, 'f', 2));
			}
		}
		//result defect score and Y-Offset
	}
}

//Startet und beendet die Kamerasimulation
void ImageData::StartCameraSimulation(bool Start)
{
	if (GetCameraSimulation())
	{
		if (Start)
		{
			if (!GetCameraSimulation()->isRunning())
				 GetCameraSimulation()->StartSimulation();
		}
		else
		{
			if (GetCameraSimulation()->isRunning())
				GetCameraSimulation()->WaitForFinshed();
		}
	}
}


void ImageData::DetachSharedMemoryInPrintchecktask()
{
	if (GetMeasureTaskPrintCheck())
		GetMeasureTaskPrintCheck()->DetachSharedMemoryInPrintchecktask();
}


int ImageData::GetSimulationCameraIntervalInms()
{
	int rv = 120;
	if (GetMainAppPrintCheck())
	{
		double temp=GetCalculatetCameraAcquisitionLineRate(GetMainAppPrintCheck()->GetSpeedSimulationInMPerMin())/1000.0;//  1/ms
		if (temp > 0.0)
		{
			int NumberLinesFromCamera = GetMainAppPrintCheck()->GetCameraHeightInPixel(GetCameraIndex());
			double TimePerLineInms = 1.0 / temp;
			rv = static_cast<int>(TimePerLineInms * NumberLinesFromCamera);
		}
    }
	return rv;
}


double ImageData::GetCalculatetCameraAcquisitionLineRate(double SpeedInmPerMin)
{
	double rv = 0.0;
	if (GetMainAppPrintCheck())
	{
		double SpeedInMMPerms      = SpeedInmPerMin / 60.0;
		double PixelSizeInMM       = GetPixelSize();

		if (PixelSizeInMM > 0.0)
		{
			rv = (SpeedInMMPerms / PixelSizeInMM) * 1000.0;//   1/sec [Herz]
		}
     }
	return rv;
}


void ImageData::SlotSetCameraStatus(const QString &Text,bool Simulation)
{
	if (GetMainAppPrintCheck())
	{
		GetMainAppPrintCheck()->SetCameraStatus(GetCameraIndex(), Text, Simulation);
	}
}


void ImageData::SlotStartToggelTimerShowLiveView()
{
	if (GetToggelTimeLiveAndMeasureViewInms() > 0)
	{
		SetTabWidgetLiveOrErrorImage(TAB_INDEX_ERROR_IMAGE);
		QTimer::singleShot(GetToggelTimeLiveAndMeasureViewInms(), this, SLOT(SlotShowLiveImageTimeoutToggelTime()));
	}
}


void ImageData::SlotShowLiveImageTimeoutToggelTime()
{
    	if (!IsCheckedShowOnlyErrorImage())
			SetTabWidgetLiveOrErrorImage(TAB_INDEX_LIVE_IMAGE);
}


void ImageData::StartTimerMeasureFullInspectionTime()
{
	m_TimerMeasureFullInspectionTime.start();
	m_FullInspectionTimeInms = 0.0;
}


void ImageData::StopTimerMeasureFullInspectionTime()
{
	m_FullInspectionTimeInms = m_TimerMeasureFullInspectionTime.nsecsElapsed() / 1000000.0;
	emit SignalShowInspectionTime(m_FullInspectionTimeInms);
}


double ImageData::GetPixelSize()
{
	if (GetMainAppPrintCheck())
	{
		if (m_CameraIndex == CAMERA_TOP_INDEX)
			return GetMainAppPrintCheck()->GetPixelSizeCameraTopInMMPerPixel();
		else
			return GetMainAppPrintCheck()->GetPixelSizeCameraBotInMMPerPixel();
	}
	else
		return 0.06;
}


bool ImageData::GetTopCameraIsFirst()
{
	if (GetMainAppPrintCheck())
	{
		return GetMainAppPrintCheck()->GetTopCameraIsFirst();
	}
	else
		return false;
}


void  ImageData::SetPixelSize(double set)
{
	if (GetMainAppPrintCheck())
	{
		if (m_CameraIndex == CAMERA_TOP_INDEX)
			GetMainAppPrintCheck()->SetPixelSizeCameraTopInMMPerPixel(set);
		else
			GetMainAppPrintCheck()->SetPixelSizeCameraBotInMMPerPixel(set);
	}
}


int  ImageData::GetNumberFormatsInReferenceImageView()
{
	if (GetMainAppPrintCheck())
		return GetMainAppPrintCheck()->GetNumberFormatsInReferenceImageView();
	else
		return 2;
}


void ImageData::SetNumberFormatsInReferenceImageView(int set)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetNumberFormatsInReferenceImageView(set);
}


void ImageData::LoadAllMeasuringSettings()
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->LoadSettings();
	
	if (GetMatchingShapeBased())
		GetMatchingShapeBased()->LoadSettings();

     if(GetMatchingFormatWindow())
		GetMatchingFormatWindow()->LoadSettings();

	 if(GetPrintLineDetector())
		 GetPrintLineDetector()->LoadSettings();
}


HalconCpp::HImage  *ImageData::GetHalconRGBResultImage()
{
	if (GetMeasureTaskPrintCheck())
		return GetMeasureTaskPrintCheck()->GetHalconRGBResultImage();
	else
		return NULL;
}


void ImageData::ShowDetailResults()
{
	QList<QScreen *> ListScreens = QGuiApplication::screens();

	if (ListScreens.count() > 0 && GetWidgetDetailResults())
	{
		if (GetCameraIndex() == CAMERA_TOP_INDEX)
		{
			GetWidgetDetailResults()->setGeometry(0, STANDARD_Y_DIALOG_POSITION + 250, ListScreens.at(0)->availableGeometry().width(), ListScreens.at(0)->availableGeometry().height() / 2.2);
			GetWidgetDetailResults()->setWindowTitle(tr("Measure Results Camera Top"));
		}
		else
		{
			GetWidgetDetailResults()->setGeometry(0, STANDARD_Y_DIALOG_POSITION + 500 + 150, ListScreens.at(0)->availableGeometry().width(), ListScreens.at(0)->availableGeometry().height() / 2.2);
			GetWidgetDetailResults()->setWindowTitle(tr("Measure Results Camera Bot"));
		}
		GetWidgetDetailResults()->setWindowFlags(Qt::WindowStaysOnTopHint);
		GetWidgetDetailResults()->show();
    }
}


void ImageData::GenerateImageTimeStampList(QList< ImageLineInformation> &ListLineInformation, int NumberLines, unsigned __int64   TimeStampInMuSecLastImageLine)
{
	int CameraLinesPerSecond = GetMainAppPrintCheck()->GetAcquisitionLineRate(GetCameraIndex());
	unsigned __int64 LineDurationInMuSec = (unsigned __int64)round(1.0 / CameraLinesPerSecond * 1000 * 1000);

	for (int i = NumberLines; i > 0; --i)
	{
		ImageLineInformation li;
		li.m_TimeStampInMuSec = TimeStampInMuSecLastImageLine - (i * LineDurationInMuSec);
		ListLineInformation.push_back(li);
	}

	/*bool ReadDirectionLeftToRight;

	if (GetCameraIndex() == CAMERA_TOP_INDEX)//Prüfe Orientierung der Kamera
		ReadDirectionLeftToRight = !GetTopCameraIsFirst();
	else
		ReadDirectionLeftToRight = GetTopCameraIsFirst();
	if (ReadDirectionLeftToRight)
	{
		for (int i = NumberLines; i > 0; --i)
		{
			ImageLineInformation li;
			li.m_TimeStampInMuSec = TimeStampInMuSecLastImageLine - (i * LineDurationInMuSec);
			ListLineInformation.push_back(li);
		}
	}
	else
	{
		for (int i = 0; i < NumberLines; i++)
		{
			ImageLineInformation li;
			li.m_TimeStampInMuSec = TimeStampInMuSecLastImageLine + (i * LineDurationInMuSec);
			ListLineInformation.push_back(li);
		}
	}
	*/
}


void ImageData::ResetFormatCheckData()
{
	if (GetMeasureTaskFormatCheck())
	{
		GetMeasureTaskFormatCheck()->SetFormatNotFoundCounter(0);
		GetMeasureTaskFormatCheck()->SetLastXPosInHoseCoordinates(0.0);//zur berechnung der Formatlänge können nur zwei direkt hintereianderliegende Formate verwendet werden. daher wird der Wert bei Schluchende/Anfange der Wert auf Null gesetzt
	}
}


int ImageData::GetNumberHose()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetMainGUIPrintCheck())
		return GetMainAppPrintCheck()->GetMainGUIPrintCheck()->GetNumberHose();
	else
		return 0;
}


void ImageData::ShowImageCounter(double Value)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->ShowImageCounter(GetCameraIndex(), Value);
}


void ImageData::SlotShowInspectionTime(double Value)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetInspectionTime(GetCameraIndex(),Value);
}

//not in use
void ImageData::ShowMeanDefectScore(double Value)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetMeanDefectScore(GetCameraIndex(),Value);
}

//not in use
void ImageData::ShowMaxDefectScore(double Value)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetMaxDefectScore(GetCameraIndex(), Value);
}


void ImageData::SlotShowCameraFramesPerSecond(double value)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetCameraFramesPerSecond(GetCameraIndex(), value);
}

//not in use
void ImageData::SetMaxCenterOffset(double Value)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetMaxCenterOffset(GetCameraIndex(), Value);
}


void ImageData::ShowInspectionErrorText(const QString &ErrorText,bool Error)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->SetSignalInspectionErrorText(GetCameraIndex(), ErrorText,Error);
}


void ImageData::SetTabWidgetLiveOrErrorImage(int TabIndex)
{
	if (GetMainAppPrintCheck())
	{
		GetMainAppPrintCheck()->GetMainGUIPrintCheck()->SetTabWidget(GetCameraIndex(), TabIndex);
	}
}


bool ImageData::IsCheckedShowOnlyErrorImage()
{
	bool rv = false;
	if (GetMainAppPrintCheck())
		rv=GetMainAppPrintCheck()->IsCheckedShowOnlyErrorImage(GetCameraIndex());
	return rv;
}


void ImageData::DeleteSelectedInspectionWindow()
{
	if (GetReferenceImageView())
		GetReferenceImageView()->DeleteSelectedRect();
}


void ImageData::ShowSelectedRectKoordinates(QRectF &rect)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->ShowSelectedRectKoordinates(GetCameraIndex(), rect);
}


int ImageData::GetToggelTimeLiveAndMeasureViewInms()
{
	if (GetMainAppPrintCheck())
	{
		return GetMainAppPrintCheck()->GetToggelTimeLiveAndMeasureViewInms();
	}
	return 5000;
}


double ImageData::GetDisplayZoomFactorFormatImage()
{
	if (GetMainAppPrintCheck())
	{
		return GetMainAppPrintCheck()->GetDisplayZoomFactorFormatImage();
	}
	return 1.0;
}


double ImageData::GetDisplayZoomFactorCameraImage()
{
	if (GetMainAppPrintCheck())
	{
		return GetMainAppPrintCheck()->GetDisplayZoomFactorCameraImage();
	}
	return 1.0;
}


ProductData *ImageData::GetCurrentProductData()
{
	if (GetMainAppPrintCheck())
		return GetMainAppPrintCheck()->GetCurrentProductData();
	else
		return NULL;
}


void ImageData::SetEventGenerateReferenceData()
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->GenerateReferenceData();
}


void ImageData::EventFromPLCTubeEndIsReached()
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->EventFromPLCTubeEndIsReached();
}


void ImageData::LoadAndShowReferenceImageFromDisk()
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->LoadAndShowReferenceImageFromDisk();
}


void ImageData::ClearSharedMemory()
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->ClearSharedMemory();
}


void ImageData::StartInspection()
{
	if(GetMeasureTaskDetectHose())
	   GetMeasureTaskDetectHose()->StartInspection();
	if(GetMeasureTaskFormatCheck())
	   GetMeasureTaskFormatCheck()->StartInspection();
	if(GetMeasureTaskPrintCheck())
	   GetMeasureTaskPrintCheck()->StartInspection();
}


int  ImageData::WriteLogFile(const QString &data, const QString &FileName)
{
	return GetMainAppPrintCheck()->WriteLogFile(data, FileName);
}


void ImageData::NewIncommingImage(const cv::Mat &image)
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->NewIncommingImage(image);
}


void ImageData::SetEnableWriteFullHose(bool set)
{
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->SetEnableWriteFullHose(set);
}


bool ImageData::GetEnableWriteFullHose()
{
	if (GetMeasureTaskDetectHose())
		return GetMeasureTaskDetectHose()->GetEnableWriteFullHose();
	else
		return false;
}


/*double ImageData::GetPrintErrorTolInPercent()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
		return pProductData->m_PrintErrorTolInPercent;
	return 0.0;
}
*/


double ImageData::GetPositionTolInMM()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
		return pProductData->m_PositionTolInMM;
	return 0.0;
}


double ImageData::GetFormatLenghtTolInMM()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
		return pProductData->m_FormatLenghtTolInMM;
	return 0.0;
}


double ImageData::GetBlockHeightTolInMM()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
		return pProductData->m_BlockHeightTolInMM;
	return 0.0;
}


double ImageData::GetBlockWidthTolInMM()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
		return pProductData->m_BlockWidthTolInMM;
	return 0.0;
}


double ImageData::GetMaxFormatLenghtInPixel()
{
	double FormatLenghtInMM = GetMainAppPrintCheck()->GetMaxFormatLenghtInMM();
	double PixelSize = GetPixelSize();

	if (PixelSize > 0.0)
		return FormatLenghtInMM / PixelSize;
	else
		return FormatLenghtInMM / 0.06; //Fall tritt eigentlich nicht auf
}


double ImageData::GetHoseDiameterInPixel()
{
	ProductData *pProductData = GetCurrentProductData();
	double PixelSize = GetPixelSize();

	if (PixelSize > 0.0)
	{
		if (pProductData)
		{
			return pProductData->m_ProductDiameter / PixelSize;
		}
		else
			return 0.0;
	}
	else	
	    return 0.0;//Fall tritt eigentlich nicht auf
}


double ImageData::GetHoseDiameterInMM()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		return pProductData->m_ProductDiameter;
	}
	else
		return 0.0;
}


double ImageData::GetFormatLenghtInPixel()
{
	ProductData *pProductData = GetCurrentProductData();
	{
		if (pProductData && GetPixelSize()>0.0)
		{
			if (GetCameraIndex() == CAMERA_TOP_INDEX)
			    return (pProductData->m_FormatLenghtInMMTopCamera)/GetPixelSize();
			else
				return (pProductData->m_FormatLenghtInMMBotCamera)/GetPixelSize();
		}
	}
	return 2500;
}


double ImageData::GetFullHoseLenghtInPixel()
{
	double HoseLenght=GetMainAppPrintCheck()->GetMaxHoseLenghtInMM();

	if (GetPixelSize() > 0.0 && HoseLenght > 0.0)
	   return HoseLenght / GetPixelSize();//ca. 10 Meter
	else
	   return 11000.0 / 0.060;
}


QRectF ImageData::GetInspectionRect(int InsepectionWindowID)
{
	ProductData *pProductData = GetCurrentProductData();
	QRectF CurrentRect;
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByID(InsepectionWindowID);
			if (pInspectionWindow)
			{
				CurrentRect = pInspectionWindow->m_ReferenceRect;
			}
		}
	}
	return CurrentRect;
}


SubFormatData *ImageData::GetSubFormatData()
{
	if (GetCurrentProductData())
		return GetCurrentProductData()->GetSubFormat(GetCameraIndex());
	else
		return NULL;
}


QRectF ImageData::GetInspectionRectByIndex(int InsepectionWindowIndex)
{
	ProductData *pProductData = GetCurrentProductData();
	QRectF CurrentRect;
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(InsepectionWindowIndex);
			if (pInspectionWindow)
			{
				CurrentRect = pInspectionWindow->m_ReferenceRect;
			}
		}
	}
	return CurrentRect;
}


void ImageData::CalculateFormatRect()
{
	ProductData *pProductData = GetCurrentProductData();
	InspectionWindow *pInspectionWindow = NULL;
	QRectF NewRect;
	double MinXPos = 1000000.0;
	double MinYPos = 1000000.0;
	double MaxXPos = 0.0;
	double MaxYPos = 0.0;
	bool RectOk = false;
	int InspectionWindowIDFormatRect = 0;

	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			for (int i = 0; i < pSubFormatData->GetNumberInspectionWindows(); i++)
			{
				pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(i);
				if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
				{
					if (pInspectionWindow->m_ReferenceRect.topLeft().x() < MinXPos)
						MinXPos = pInspectionWindow->m_ReferenceRect.topLeft().x();
					if (pInspectionWindow->m_ReferenceRect.topLeft().y() < MinYPos)
						MinYPos = pInspectionWindow->m_ReferenceRect.topLeft().y();
					if (pInspectionWindow->m_ReferenceRect.bottomRight().x() > MaxXPos)
						MaxXPos = pInspectionWindow->m_ReferenceRect.bottomRight().x();
					if (pInspectionWindow->m_ReferenceRect.bottomRight().y() > MaxYPos)
						MaxYPos = pInspectionWindow->m_ReferenceRect.bottomRight().y();
					RectOk = true;
				}
	    	}
			if (RectOk)
			{
				int MaxHeight, MaxWidth;
				int Rim = 16;
				GetMeasureTaskDetectHose()->GetReferenceSize(MaxWidth, MaxHeight);
				//links,rechts, oben und unten etwas Puffer wegen Randproblem beim Matching
				MinXPos = MinXPos - Rim;
				if (MinXPos < 0)MinXPos = 0;
				MaxXPos = MaxXPos + Rim;
				if (MaxXPos >= MaxWidth)MaxXPos = MaxWidth - 1;

				MinYPos = MinYPos - Rim;
				if (MinYPos < 0)MinYPos = 0;
				MaxYPos = MaxYPos + Rim;
				if (MaxYPos >= MaxHeight)MaxYPos = MaxHeight - 1;
				
				NewRect.setTopLeft(QPointF(MinXPos, MinYPos));
				NewRect.setBottomRight(QPointF(MaxXPos, MaxYPos));
				pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(InspectionWindowIDFormatRect);
				if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID == INSPECTION_ID_FORMAT_WINDOW)
				{
					pInspectionWindow->m_ReferenceRect = NewRect;
					CalculateRectPosRelatetToFormatRect(pSubFormatData,NewRect);
				}
			}
		}
	}
}

//Wird benötigt bei der Druckinspektion um den Suchbereich für die einzelenen Blöcke einzuschräken
void ImageData::CalculateRectPosRelatetToFormatRect(SubFormatData *pSubFormatData,QRectF &FormatRect)
{
	double diffx,XRim = 16.0;
	double x, y, w, h;
	QRectF ROIRectRelatetToMasterRect;
	InspectionWindow *pInspectionWindow = NULL;

	for (int i = 0; i < pSubFormatData->GetNumberInspectionWindows(); i++)
	{
		pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(i);
		if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID > INSPECTION_ID_FORMAT_WINDOW)
		{
			y = 0.0;
			h = FormatRect.height();
			w = pInspectionWindow->m_ReferenceRect.width() + 2.0 *XRim;
			x = pInspectionWindow->m_ReferenceRect.x() - XRim;
			if (x < FormatRect.topLeft().x())
			{
				diffx = FormatRect.topLeft().x() - x;
				w = w - diffx;
				x = FormatRect.topLeft().x();
			}

			if ((x + w) > FormatRect.bottomRight().x())
			{
				diffx = (x + w) - FormatRect.bottomRight().x();
				w = w - diffx;
			}

			x = x - FormatRect.topLeft().x();
			ROIRectRelatetToMasterRect.setX(x);
			ROIRectRelatetToMasterRect.setY(y);
			ROIRectRelatetToMasterRect.setWidth(w);
			ROIRectRelatetToMasterRect.setHeight(h);

			pInspectionWindow->m_ROIRectRelatetToFormatRect = ROIRectRelatetToMasterRect;
    	}
	}
}


QString ImageData::GetLocationProductData()
{
	ProductData *pProductData = GetCurrentProductData();
	QString Location;
	if (pProductData)
		Location = pProductData->GetLocationProductData();
	return Location;
}


QString ImageData::GetReferenceLocation()
{
	ProductData *pProductData = GetCurrentProductData();
	QString Location;
	if (pProductData)
	{
		if (GetCameraIndex() == CAMERA_TOP_INDEX)
			Location = pProductData->GetLocationCameraTopData();
		else
			Location = pProductData->GetLocationCameraBotData();
	}
	return Location;
}


InspectionWindow *ImageData::GetInspectionWindowByID(int InsepectionWindowID)
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			return pSubFormatData->GetInspectionWindowByID(InsepectionWindowID);
		}
	}
	return NULL;
}


InspectionWindow  *ImageData::GetInspectionWindowHoseDetection()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			return pSubFormatData->GetInspectionWindowHoseDetection();
		}
	}
	return NULL;
}


int ImageData::GetNumberInspectionWindows()
{
	int rv = 0;
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			return pSubFormatData->GetNumberInspectionWindows();
		}
	}
	return rv;
}


InspectionWindow *ImageData::GetInspectionWindowByIndex(int InsepectionWindowIndex)
{
	ProductData *pProductData = GetCurrentProductData();
	QRectF CurrentRect;
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			return pSubFormatData->GetInspectionWindowByIndex(InsepectionWindowIndex);
		}
	}
	return NULL;
}


void ImageData::AddNewInspectionRect(InspectionWindow *pInspectionWindow)
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			int newID = pSubFormatData->AddNewInspectionWindow(pInspectionWindow);
			if (GetReferenceImageView())
			{
				GetReferenceImageView()->SetMeasureWindowRectGraphicsItem();
				GetReferenceImageView()->DrawAllReferenceRects();
			}
		}
		else
		{
			if (pInspectionWindow)
				delete pInspectionWindow;
		}
	}
	else
	{
		if (pInspectionWindow)
			delete pInspectionWindow;
	}
}


void ImageData::RemoveInspectionRect(int InspectionID)
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			pSubFormatData->RemoveInspectionWindow(InspectionID);
		}
	}
}


void ImageData::RemoveNotValidInspectionWindow()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			for (int i = 0; i < pSubFormatData->GetNumberInspectionWindows(); i++)
			{
				InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(i);
				if (pInspectionWindow && pInspectionWindow->m_InspectionWindowID> INSPECTION_ID_FORMAT_WINDOW)
				{
					if (!pInspectionWindow->m_HaveReferenceData)
					{
						pSubFormatData->RemoveInspectionWindow(pInspectionWindow->m_InspectionWindowID);
						i = 0;
					}
				}
			}
		}
	}
}


void ImageData::WriteProductData()
{
	QString ErrorMsg;
	if (GetCurrentProductData())
		GetCurrentProductData()->WriteProductData(ErrorMsg);
}


void ImageData::SetInspectionRect(QRectF &NewRect, int InsepectionWindowID)
{
	ProductData *pProductData = GetCurrentProductData();
	QRectF CurrentRect;
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByID(InsepectionWindowID);
			if (pInspectionWindow)
			{
				double w = static_cast<int>((NewRect.width() / 2.0 + 0.5)) * 2.0;
				double h = static_cast<int>((NewRect.height() / 2.0 + 0.5)) * 2.0;
				NewRect.setWidth(w);
				NewRect.setHeight(h);
				pInspectionWindow->m_ReferenceRect = NewRect;
			}
		}
	}
}


void ImageData::SetInspectionRectByIndex(QRectF &NewRect, int InsepectionWindowIndex)
{
	ProductData *pProductData = GetCurrentProductData();
	QRectF CurrentRect;
	if (pProductData)
	{
		SubFormatData *pSubFormatData = pProductData->GetSubFormat(GetCameraIndex());
		if (pSubFormatData)
		{
			InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(InsepectionWindowIndex);
			if (pInspectionWindow)
			{
				double w = static_cast<int>((NewRect.width() / 2.0 + 0.5)) * 2.0;
				double h = static_cast<int>((NewRect.height() / 2.0 + 0.5)) * 2.0;
				NewRect.setWidth(w);
				NewRect.setHeight(h);
				pInspectionWindow->m_ReferenceRect = NewRect;
			}
		}
	}
}


void ImageData::WaitForFinshed()
{
	if (isRunning())
	{//thread läuft noch
		QMutex mutex;
		m_TerminateSaveResults = true;
		m_WaitConditionSaveResults.wakeAll();
		mutex.lock();
		m_WaitConditionWaitForFinshed.wait(&mutex, 1000);
	}
	if (GetCameraSimulation())
		GetCameraSimulation()->WaitForFinshed();
	if (GetMeasureTaskFormatCheck())
		GetMeasureTaskFormatCheck()->WaitForFinshed();
	if (GetMeasureTaskPrintCheck())
		GetMeasureTaskPrintCheck()->WaitForFinshed();
	if (GetMeasureTaskDetectHose())
		GetMeasureTaskDetectHose()->WaitForFinshed();
}


void ImageData::GetDisplayZoomedSizeFormatImage(int &DisplayWidth, int &DisplayHeight)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->GetDisplayZoomedSizeFormatImage(DisplayWidth, DisplayHeight);
}


void ImageData::GetDisplayZoomedSizeCameraImage(int &DisplayWidth, int &DisplayHeight)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->GetDisplayZoomedSizeCameraImage(DisplayWidth, DisplayHeight);
}


int ImageData::GetSaveErrorImagePoolCondition()
{
	if (GetMainAppPrintCheck())
		return GetMainAppPrintCheck()->GetSaveErrorImagePoolCondition();
	else
		return SAVE_FORMAT_IMAGE_NO_IMAGES;
}


QString ImageData::GetResultLocation()
{
	if (GetCameraIndex() == CAMERA_TOP_INDEX)
		return GetMainAppPrintCheck()->GetResultsLocationCameraTop();
	else
		return GetMainAppPrintCheck()->GetResultsLocationCameraBot();
}


void ImageData::AppendInspectionResultsForResultView(const ImageMetaData &Image)
{
	if (m_WidgetDetailResults && m_WidgetDetailResults->isVisible())
	{
	  InspectionWindow Newresults;
	
	  m_ListResultsInspectionWindows.clear();
	  for (int i = 0; i < Image.m_ListInspectionWindowResults.count(); i++)
	  {
		Newresults = Image.m_ListInspectionWindowResults.at(i);
		m_ListResultsInspectionWindows.insert(Newresults.m_InspectionWindowID, Newresults);
	  }
	  Newresults = *GetInspectionWindowHoseDetection();
	  m_ListResultsInspectionWindows.insert(Newresults.m_InspectionWindowID, Newresults);
	  ((TableWidgetSubFormatResults *)(m_WidgetDetailResults->GetWidget(0)))->UpdateResults();
	}
}

//not in use
void ImageData::SetAdditionalResultData(QString &data)
{
	if (GetWidgetDetailResults() && GetWidgetDetailResults()->GetWidget(1))
	{
		((QListWidget*)(GetWidgetDetailResults()->GetWidget(1)))->addItem(data);// QString("Format Lenght:%1").arg(Image.m_ListInspectionWindowResults.at(i).m_Results.m_FormatLengthInPix));
		if (((QListWidget*)(GetWidgetDetailResults()->GetWidget(1)))->count() > 20)
			((QListWidget*)(GetWidgetDetailResults()->GetWidget(1)))->takeItem(0);
	}
}

//Thread zum speichern der Bild-  und Messdaten
void ImageData::run()
{
	QMutex Mutex;

	
	while (!m_TerminateSaveResults)
	{
		Mutex.lock();
		if (m_QQueueSaveImageAndResults.isEmpty())
			m_WaitConditionSaveResults.wait(&Mutex);
		if (m_TerminateSaveResults)break;//Abbruch, Programm wird beendet
		Mutex.unlock();

		while (!m_QQueueSaveImageAndResults.isEmpty())
		{
			if (m_MutexSaveImageAndResults.tryLock())
			{
				ImageMetaData ResultAndImage = m_QQueueSaveImageAndResults.dequeue();
				m_MutexSaveImageAndResults.unlock();
				SaveInspectionResults(ResultAndImage);
			}
			else
				msleep(0);
		}
	}
	m_WaitConditionWaitForFinshed.wakeAll();
}


void ImageData::AppendInspectionResultsForStorage(ImageMetaData &ResultAndImage)
{
	if (isRunning())
	{
		m_MutexSaveImageAndResults.lock();
		m_QQueueSaveImageAndResults.enqueue(ResultAndImage);
		m_MutexSaveImageAndResults.unlock();
		m_WaitConditionSaveResults.wakeAll();
	}
	else
	{
		m_MutexSaveImageAndResults.lock();
		while (!m_QQueueSaveImageAndResults.isEmpty())
			    m_QQueueSaveImageAndResults.dequeue();
		m_MutexSaveImageAndResults.unlock();
    }
}


void ImageData::SaveInspectionResults(ImageMetaData &ResultAndImage)
{
	QString HeaderCSV, ResultLine, DataPathAndFileName, ImagePathAndFileName, LocationMeasuringResults,ParentLocaton;
	QString Sperator = ",";
	QStringList ListResults;
	InspectionWindow InspectionResults;
	QString DirResults = GetResultLocation();
	QString CurrentDate = QDateTime::currentDateTime().date().toString();
	QString CurrentTime = QDateTime::currentDateTime().time().toString("hh mm ss zzz");
	bool QualityBadIn = false;
	bool QualityGoodIn = false;
	

	switch (GetSaveErrorImagePoolCondition())
	{
	case SAVE_FORMAT_IMAGE_ALL_IMAGES:
		ParentLocaton = DirResults + QString("/ImagePool");
		QDir().mkdir(ParentLocaton);
	    break;
	case SAVE_FORMAT_IMAGE_ONLY_BAD_IMAGES:
		ParentLocaton = DirResults + QString("/BadImagePool");
		QDir().mkdir(ParentLocaton);
    	break;
	case SAVE_FORMAT_IMAGE_ONLY_GOOD_IMAGES:
		ParentLocaton = DirResults + QString("/GoodImagePool");
		QDir().mkdir(ParentLocaton);
		break;
	default:
		return;
	}
	CheckImageDir(ParentLocaton);//FIFO reduce directory 
	LocationMeasuringResults = ParentLocaton + QString("/DataSet[%1 %2]").arg(CurrentDate).arg(CurrentTime);
	QDir().mkdir(LocationMeasuringResults);
	ImagePathAndFileName = LocationMeasuringResults + QString("/") + QString("FormatImage.bmp");
	DataPathAndFileName  = LocationMeasuringResults + QString("/") + QString("FormatResult.csv");
	

	QFile ResultCSV(DataPathAndFileName);

	if (QFileInfo::exists(DataPathAndFileName))
		ResultCSV.remove();
	for (int i = 0; i < m_HeaderListInspectionResults.count(); i++)
		HeaderCSV += m_HeaderListInspectionResults.at(i) + Sperator;
	HeaderCSV.remove(HeaderCSV.size() - 1, 1);
	WriteInspectionResults(DataPathAndFileName, HeaderCSV, false);

	switch (GetSaveErrorImagePoolCondition())
	{

	case SAVE_FORMAT_IMAGE_ALL_IMAGES:
		for (int i = 0; i < ResultAndImage.m_ListInspectionWindowResults.count(); i++)
		{
			InspectionResults = ResultAndImage.m_ListInspectionWindowResults.at(i);
			GetListMeasurementResults(&InspectionResults, ListResults);
			ResultLine = "";
			for (int k = 0; k < ListResults.count(); k++)
				ResultLine += ListResults.at(k) + Sperator;
			ResultLine.remove(ResultLine.size() - 1, 1);
			WriteInspectionResults(DataPathAndFileName, ResultLine, false);
		}
		ResultAndImage.m_Pixmap.save(ImagePathAndFileName);

		break;
	case SAVE_FORMAT_IMAGE_ONLY_BAD_IMAGES:
		for (int i = 0; i < ResultAndImage.m_ListInspectionWindowResults.count(); i++)
		{
			InspectionResults = ResultAndImage.m_ListInspectionWindowResults.at(i);
			if (!InspectionResults.IsQualityOk())
			{
				GetListMeasurementResults(&InspectionResults, ListResults);
				ResultLine = "";
				for (int k = 0; k < ListResults.count(); k++)
					ResultLine += ListResults.at(k) + Sperator;
				ResultLine.remove(ResultLine.size() - 1, 1);
				WriteInspectionResults(DataPathAndFileName, ResultLine, false);
				QualityBadIn = true;
			}
		}
		if (QualityBadIn)
			ResultAndImage.m_Pixmap.save(ImagePathAndFileName);
		break;
	case SAVE_FORMAT_IMAGE_ONLY_GOOD_IMAGES:
		for (int i = 0; i < ResultAndImage.m_ListInspectionWindowResults.count(); i++)
		{
			InspectionResults = ResultAndImage.m_ListInspectionWindowResults.at(i);
			if (InspectionResults.IsQualityOk())
			{
				GetListMeasurementResults(&InspectionResults, ListResults);
				ResultLine = "";
				for (int k = 0; k < ListResults.count(); k++)
					ResultLine += ListResults.at(k) + Sperator;
				ResultLine.remove(ResultLine.size() - 1, 1);
				WriteInspectionResults(DataPathAndFileName, ResultLine, false);
				QualityGoodIn = true;
			}
		}
		if (QualityGoodIn)
			ResultAndImage.m_Pixmap.save(ImagePathAndFileName);
		break;
	}
}


void ImageData::CheckImageDir(QString &Dir)
{
	QDir dir(Dir);
    QStringList totalDirs;
	
	totalDirs = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Time | QDir::Reversed);
	int diff  = totalDirs.count() - GetMainAppPrintCheck()->GetMaxNumberOfImagesInDir() + 1;

	if(diff>=0)
	{
		for (int i = 0; i < diff; i++)
		{
			QDir dir(Dir + QString("/") + totalDirs[i]);
			dir.removeRecursively();//Lösche ältesten datensätze FIFO Prinzip
		}
	}
}


int ImageData::WriteInspectionResults(QString &FileName, QString &data, bool WithDateAndTime)
{
	int retVal = ERROR_CODE_NO_ERROR;
	QString Data;
	QFile CurrentFile;
	QString Seperator = "|";

	CurrentFile.setFileName(FileName);
	if (WithDateAndTime)
		Data = QDateTime::currentDateTime().date().toString("dd.MM.yy") + Seperator + QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz") + Seperator + data;
	else
		Data = data;
	if (CurrentFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{//write data
		QTextStream os(&CurrentFile);
		os << Data << "\r\n";
		CurrentFile.close();
	}
	else
		retVal = ERROR_CODE_ANY_ERROR;
		
	return retVal;
}


void ImageData::CopyROIImage(cv::Rect &ROIRect, cv::Mat &Source, cv::Mat &ROIImage)
{
	cv::Mat ROIImageRef(Source, ROIRect);

	ROIImage = cv::Mat::zeros(ROIRect.height, ROIRect.width, CV_8UC1);
	ROIImageRef.copyTo(ROIImage(cv::Rect(0, 0, ROIRect.width, ROIRect.height)));
}


QImage ImageData::CopyHalconImageIntoQtImage(HalconCpp::HImage  &HalconImage, int NumberChannelsQtImage)
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



