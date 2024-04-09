#include "MainAppPrintCheck.h"
#include "LiveImageView.h"
#include "ErrorImageView.h"
#include "CameraImageView.h"
#include "ImageData.h"
#include "bremsschlauchkontrolleplugin.h"
#include "GlobalConst.h"
#include "ProductData.h"
#include "MainGUIPrintCheck.h"
#include "WidgetEditReferenceData.h"
#include "WidgetEditProductData.h"
#include "WidgetEditReferenceImageData.h"
#include "MeasureTaskDetectHose.h"
#include "ProductDialog.h"
#include "WidgetEditCustomSettings.h"
#include "WidgetEditGeneralSettings.h"
#include "WidgetEditMeasureParameter.h"


MainAppPrintCheck::MainAppPrintCheck(BremsSchlauchKontrollePlugin *Parent) : QObject()
, m_BremsSchlauchKontrollePlugin(NULL)
, m_MainGUIPrintCheck(NULL)
, m_WidgetEditReferenceData(NULL)
, m_WidgetEditProductData(NULL)
, m_ToggelTimeLiveAndMeasureViewInms(0)
, m_NumberFormatsInReferenceImageView(2)
, m_SaveErrorImagePoolCondition(SAVE_FORMAT_IMAGE_NO_IMAGES)
, m_SimulationCameraTopOn(false)
, m_SimulationCameraBotOn(false)
, m_DisplayZoomFactorFormatImage(0.65)
, m_DisplayZoomFactorCameraImage(0.25)
, m_PixelSizeCameraTopInMMPerPixel(0.066)
, m_PixelSizeCameraBotInMMPerPixel(0.066)
, m_PopupDialogProductDialog(NULL)
, m_TopCameraIsFirst(false)
, m_CameraCenterOffsetInMM(26.5)
, m_TransmissionDistanceCameraTopPLCInMM(500.0)
, m_TransmissionDistanceCameraBotPLCInMM(500.0)
, m_SimulationPLC(NULL)
, m_WidgetEditCustomSettings(NULL)
, m_CheckIsDoWorkRunning(NULL)
, m_WidgetEditMeasureParameterCameraTop(NULL)
, m_WidgetEditMeasureParameterCameraBot(NULL)
, m_SpeedSimulationInMPerMin(30.0)
, m_PLC(NULL)
, m_WaitTimeAfterTubEndCameraTopInms(600)
, m_WaitTimeAfterTubEndCameraBotInms(600)
, m_MaxErrorsBehindEachOther(5)
, m_IntervallBetweenTwoErrors(1000)
, m_InspectionIsStoppedCameraTop(false)
, m_InspectionIsStoppedCameraBot(false)
, m_MaxFormatLenghtInMM(200.0)
, m_PLCSimulationOn(false)
, m_DisableShowDebugInfoMeasureResults(false)
, m_ShowDebugInfoOnlyError(false)
, m_ProgramHasAlreadyEnded(false)
, m_RestartApplicationWhenCameraTimeout(false)
{
	QString RootLocationProductData;
	bool CameraSimulationOnStartOn = false;//sollte immer auf false stehen nur für Testzwecke auf true setzen
	m_PLCSimulationOn = false;//sollte immer auf false stehen nur für Testzwecke auf true setzen
	
	m_PLC = std::shared_ptr<MainLogic>(new MainLogic(this));
	m_BremsSchlauchKontrollePlugin = Parent;

	//Verzeichnisse erstellen
	RootLocationProductData = "d:/ProductData";
	m_LogFileLocation = RootLocationProductData + QString("/") + "LogFiles";
	QDir().mkpath(m_LogFileLocation);
	m_ProductLocation = RootLocationProductData + QString("/") + "ProductFiles";
	QDir().mkpath(m_ProductLocation);
	m_ImageLocation = "d:/Images";
	QDir().mkpath(m_ImageLocation);
	m_VideoFileLocationCameraTop = m_ImageLocation + "/HoseVideosCameraTop";
	QDir().mkpath(m_VideoFileLocationCameraTop);
	m_VideoFileLocationCameraBot = m_ImageLocation + "/HoseVideosCameraBot";
	QDir().mkpath(m_VideoFileLocationCameraBot);
	m_ResultsLocationCameraTop = m_ImageLocation + "/ResultsCameraTop";
	QDir().mkpath(m_ResultsLocationCameraTop);
	m_ResultsLocationCameraBot = m_ImageLocation + "/ResultsCameraBot";
	QDir().mkpath(m_ResultsLocationCameraBot);
	
    m_DisableShowDebugInfoMeasureResults = m_BremsSchlauchKontrollePlugin->GetPreference("DisableShowDebugInfoMeasureResults").toBool();
    m_BremsSchlauchKontrollePlugin->SetPreference("DisableShowDebugInfoMeasureResults", m_DisableShowDebugInfoMeasureResults);

    m_ShowDebugInfoOnlyError = m_BremsSchlauchKontrollePlugin->GetPreference("ShowDebugInfoOnlyError").toBool();
    m_BremsSchlauchKontrollePlugin->SetPreference("ShowDebugInfoOnlyError", m_ShowDebugInfoOnlyError);

	m_RestartApplicationWhenCameraTimeout = m_BremsSchlauchKontrollePlugin->GetPreference("RestartApplicationWhenCameraTimeout").toBool();
	m_BremsSchlauchKontrollePlugin->SetPreference("RestartApplicationWhenCameraTimeout",m_RestartApplicationWhenCameraTimeout);

	//hier Einstellungen die über die GUI erreichbar sind
	m_NumberFormatsInReferenceImageView = m_BremsSchlauchKontrollePlugin->GetPreference("NumberFormatsInReferenceImageView").toInt();
	if (m_NumberFormatsInReferenceImageView == 0)
		m_NumberFormatsInReferenceImageView = 2;

	m_SaveErrorImagePoolCondition = m_BremsSchlauchKontrollePlugin->GetPreference("SaveErrorImagePoolCondition").toInt();
	if (m_SaveErrorImagePoolCondition == 0)
		m_SaveErrorImagePoolCondition = SAVE_FORMAT_IMAGE_NO_IMAGES;

	m_PixelSizeCameraTopInMMPerPixel = m_BremsSchlauchKontrollePlugin->GetPreference("PixelSizeCameraTopInMMPerPixel").toDouble();
	if (m_PixelSizeCameraTopInMMPerPixel == 0.0)
		m_PixelSizeCameraTopInMMPerPixel = 0.060;
	SetPixelSizeCameraTopInMMPerPixel(m_PixelSizeCameraTopInMMPerPixel);

    m_PixelSizeCameraBotInMMPerPixel = m_BremsSchlauchKontrollePlugin->GetPreference("PixelSizeCameraBotInMMPerPixel").toDouble();
	if(m_PixelSizeCameraBotInMMPerPixel == 0.0)
	   m_PixelSizeCameraBotInMMPerPixel = 0.060;
	SetPixelSizeCameraBotInMMPerPixel(m_PixelSizeCameraBotInMMPerPixel);

	m_ToggelTimeLiveAndMeasureViewInms = m_BremsSchlauchKontrollePlugin->GetPreference("ToggelTimeLiveAndMeasureViewInms").toInt();
		
	m_CameraCenterOffsetInMM = m_BremsSchlauchKontrollePlugin->GetPreference("CameraCenterOffsetInMM").toDouble();
	if (m_CameraCenterOffsetInMM == 0.0)
		m_CameraCenterOffsetInMM = 26.5;

    m_TransmissionDistanceCameraTopPLCInMM = m_BremsSchlauchKontrollePlugin->GetPreference("TransmissionDistanceCameraTopPLCInMM").toDouble();
	if (m_TransmissionDistanceCameraTopPLCInMM == 0.0)
		m_TransmissionDistanceCameraTopPLCInMM = 500.0;

	m_TransmissionDistanceCameraBotPLCInMM = m_BremsSchlauchKontrollePlugin->GetPreference("TransmissionDistanceCameraBotPLCInMM").toDouble();
	if (m_TransmissionDistanceCameraBotPLCInMM == 0.0)
		m_TransmissionDistanceCameraBotPLCInMM = 500.0;

	m_SpeedSimulationInMPerMin = m_BremsSchlauchKontrollePlugin->GetPreference("SpeedSimulationInMPerMin").toDouble();
	if (m_SpeedSimulationInMPerMin == 0.0)
		m_SpeedSimulationInMPerMin = 30.0;

	m_MaxErrorsBehindEachOther = m_BremsSchlauchKontrollePlugin->GetPreference("MaxErrorsBehindEachOther").toInt();
	if (m_MaxErrorsBehindEachOther == 0)
		m_MaxErrorsBehindEachOther = 5;

	m_IntervallBetweenTwoErrors = m_BremsSchlauchKontrollePlugin->GetPreference("IntervallBetweenTwoErrors").toInt();
	if (m_IntervallBetweenTwoErrors == 0)
		m_IntervallBetweenTwoErrors = 1000;

	m_TopCameraIsFirst = m_BremsSchlauchKontrollePlugin->GetPreference("TopCameraIsFirst").toBool();
	SetTopCameraIsFirst(m_TopCameraIsFirst);

	m_MaxNumberOfImagesInDir = m_BremsSchlauchKontrollePlugin->GetPreference("MaxNumberOfImagesInDir").toInt();
	if (m_MaxNumberOfImagesInDir == 0)
		m_MaxNumberOfImagesInDir = 100;
	m_BremsSchlauchKontrollePlugin->SetPreference("MaxNumberOfImagesInDir", QVariant(m_MaxNumberOfImagesInDir));

	m_WaitTimeAfterTubEndCameraTopInms = m_BremsSchlauchKontrollePlugin->GetPreference("WaitTimeAfterTubEndCameraTopInms").toInt();
	if (m_WaitTimeAfterTubEndCameraTopInms == 0)
		m_WaitTimeAfterTubEndCameraTopInms = 750;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("WaitTimeAfterTubEndCameraTopInms"), QVariant(m_WaitTimeAfterTubEndCameraTopInms));

	m_WaitTimeAfterTubEndCameraBotInms = m_BremsSchlauchKontrollePlugin->GetPreference("WaitTimeAfterTubEndCameraBotInms").toInt();
	if (m_WaitTimeAfterTubEndCameraBotInms == 0)
		m_WaitTimeAfterTubEndCameraBotInms = 750;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("WaitTimeAfterTubEndCameraBotInms"), QVariant(m_WaitTimeAfterTubEndCameraBotInms));

	m_MaxNumberCameraTopImagesHoseNotFound = m_BremsSchlauchKontrollePlugin->GetPreference("MaxNumberCameraTopImagesHoseNotFound").toInt();
	if (m_MaxNumberCameraTopImagesHoseNotFound == 0)
		m_MaxNumberCameraTopImagesHoseNotFound = 1;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("MaxNumberCameraTopImagesHoseNotFound"), QVariant(m_MaxNumberCameraTopImagesHoseNotFound));

	m_MaxNumberCameraBotImagesHoseNotFound = m_BremsSchlauchKontrollePlugin->GetPreference("MaxNumberCameraBotImagesHoseNotFound").toInt();
	if (m_MaxNumberCameraBotImagesHoseNotFound == 0)
		m_MaxNumberCameraBotImagesHoseNotFound = 1;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("MaxNumberCameraBotImagesHoseNotFound"), QVariant(m_MaxNumberCameraBotImagesHoseNotFound));
	   
	m_LastLoadedProduct = m_BremsSchlauchKontrollePlugin->GetPreference("LastProduct").toString();
	if (m_LastLoadedProduct.isEmpty())
		m_LastLoadedProduct = "DefaultType";

	//hier Einstellung die nicht über die GUI erreichbar sind
	m_DefaultBlockWidthInMM  = m_BremsSchlauchKontrollePlugin->GetPreference("DefaultBlockWidthInMM").toDouble();
	if (m_DefaultBlockWidthInMM == 0.0)
		m_DefaultBlockWidthInMM = 15.0;
	m_BremsSchlauchKontrollePlugin->SetPreference("DefaultBlockWidthInMM", QVariant(m_DefaultBlockWidthInMM));

	m_DefaultBlockHeightInMM = m_BremsSchlauchKontrollePlugin->GetPreference("DefaultBlockHeightInMM").toDouble();
	if (m_DefaultBlockHeightInMM == 0.0)
		m_DefaultBlockHeightInMM = 4.6;
	m_BremsSchlauchKontrollePlugin->SetPreference("DefaultBlockHeightInMM", QVariant(m_DefaultBlockHeightInMM));

	m_MaxFormatLenghtInMM = m_BremsSchlauchKontrollePlugin->GetPreference("MaxFormatLenghtInMM").toDouble();
	if (m_MaxFormatLenghtInMM == 0.0)
		m_MaxFormatLenghtInMM = 250.0;
	m_BremsSchlauchKontrollePlugin->SetPreference("MaxFormatLenghtInMM", QVariant(m_MaxFormatLenghtInMM));

	m_DisplayZoomFactorFormatImage = m_BremsSchlauchKontrollePlugin->GetPreference("DisplayZoomFactorFormatImage").toDouble();
	if (m_DisplayZoomFactorFormatImage == 0.0)
		m_DisplayZoomFactorFormatImage = 0.65;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("DisplayZoomFactorFormatImage"), QVariant(m_DisplayZoomFactorFormatImage));

	m_DisplayZoomFactorCameraImage = m_BremsSchlauchKontrollePlugin->GetPreference("DisplayZoomFactorCameraImage").toDouble();
	if (m_DisplayZoomFactorCameraImage == 0.0)
		m_DisplayZoomFactorCameraImage = 0.25;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("DisplayZoomFactorCameraImage"), QVariant(m_DisplayZoomFactorCameraImage));

	m_MaxHoseLenghtInMM = m_BremsSchlauchKontrollePlugin->GetPreference("MaxHoseLenghtInMM").toDouble();
	if (m_MaxHoseLenghtInMM == 0.0)
		m_MaxHoseLenghtInMM = 11000.0;
	m_BremsSchlauchKontrollePlugin->SetPreference(QString("MaxHoseLenghtInMM"), QVariant(m_MaxHoseLenghtInMM));

	m_ImageDataSet.append(new ImageData(this, CAMERA_TOP_INDEX));
	m_ImageDataSet.append(new ImageData(this, CAMERA_BOT_INDEX));

	//main GUI Widgets
	m_MainGUIPrintCheck                    = new MainGUIPrintCheck(this);//Hauptfenster
	m_WidgetEditReferenceData              = new WidgetEditReferenceData(this);//Fenster Referenz
	m_WidgetEditProductData                = new WidgetEditProductData(this);
	m_WidgetEditCustomSettings             = new WidgetEditCustomSettings(this);
	m_WidgetEditGeneralSettings            = new WidgetEditGeneralSettings(this);
	m_WidgetEditMeasureParameterCameraTop  = new WidgetEditMeasureParameter(this, CAMERA_TOP_INDEX);
	m_WidgetEditMeasureParameterCameraBot  = new WidgetEditMeasureParameter(this, CAMERA_BOT_INDEX);
	m_PopupDialogProductDialog             = new PopupDialogProductDialog(this, m_MainGUIPrintCheck);

	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		int camIndex =m_ImageDataSet.at(i)->GetCameraIndex();

		m_ImageDataSet.at(i)->SetCameraWidthInPixel(GetBremsSchlauchKontrollePlugin()->GetCameraWidthInPixel(camIndex));
		m_ImageDataSet.at(i)->SetCameraHeightInPixel(GetBremsSchlauchKontrollePlugin()->GetCameraHeightInPixel(camIndex));
		if (GetFormatImageView(camIndex))
			GetMainGUIPrintCheck()->AddLiveImageWidget(camIndex, (QWidget *)(GetFormatImageView(camIndex)));
		if (GetErrorImageView(camIndex))
			GetMainGUIPrintCheck()->AddErrorImageWidget(camIndex, (QWidget *)(GetErrorImageView(camIndex)));
		if (GetReferenceImageView(camIndex))
			GetWidgetEditReferenceData()->AddReferenceImageWidget((QWidget *)(GetReferenceImageView(camIndex)), camIndex);
		if (GetCameraImageView(camIndex))
			GetMainGUIPrintCheck()->AddCameraImageWidget(camIndex, (QWidget *)(GetCameraImageView(camIndex)));

		if (/*m_SimulationCameraTopOn && */camIndex == CAMERA_TOP_INDEX)
		{
			m_ImageDataSet.at(camIndex)->StartCameraSimulation(CameraSimulationOnStartOn);
			SetCameraTopSimulationOn(CameraSimulationOnStartOn);
			ShowVideoPlayerButtons(CameraSimulationOnStartOn, CAMERA_TOP_INDEX);
		}
		if (/*m_SimulationCameraBotOn && */camIndex == CAMERA_BOT_INDEX)
		{
			m_ImageDataSet.at(camIndex)->StartCameraSimulation(CameraSimulationOnStartOn);
			SetCameraBotSimulationOn(CameraSimulationOnStartOn);
			ShowVideoPlayerButtons(CameraSimulationOnStartOn, CAMERA_BOT_INDEX);
		}
		m_ImageDataSet.at(i)->CreateSharedmemoryFullHose();
	}
	
	m_WidgetEditReferenceData->setHidden(true);
	
	StartupApplication();

	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		m_ImageDataSet.at(i)->SetTabWidgetLiveOrErrorImage(TAB_INDEX_LIVE_IMAGE);
		m_ImageDataSet.at(i)->LoadAllMeasuringSettings();
	}
	connect(this, &MainAppPrintCheck::SignalSetHoseEnd, this, &MainAppPrintCheck::SlotSetHoseEnd);
	connect(this, &MainAppPrintCheck::SignalSetHoseStart, this, &MainAppPrintCheck::SlotSetHoseStart);
	connect(this, &MainAppPrintCheck::SignalSetHoseMiddle, this, &MainAppPrintCheck::SlotSetHoseMiddle);

	if (m_PLC != NULL)
	{
		connect(m_PLC.get(), &MainLogic::valueChanged, m_BremsSchlauchKontrollePlugin, &BremsSchlauchKontrollePlugin::valueChanged);
		connect(m_PLC.get(), &MainLogic::SignalLoadJob, this, &MainAppPrintCheck::SlotLoadJob);
		connect(m_PLC.get(), &MainLogic::SignalInspectionActive, this, &MainAppPrintCheck::SlotInspectionActive);
		connect(m_PLC.get(), &MainLogic::SignalResetFault, this, &MainAppPrintCheck::SlotResetFault);
		connect(m_PLC.get(), &MainLogic::SignalCurrentSpeed, this, &MainAppPrintCheck::SlotCurrentSpeedFromPLC);
		connect(m_PLC.get(), &MainLogic::SignalTubeEndReached, this, &MainAppPrintCheck::SlotTubeEndReached);
	}
	
	if (m_PLCSimulationOn)
	{
		m_SimulationPLC = new SimulationPLC(m_PLC.get());
		if (m_SimulationPLC)
		{
			m_SimulationPLC->start();
			m_PLC.get()->SetCurrentLineSpeed(GetSpeedSimulationInMPerMin());
		}
	}
	startTimer(1000);//starte timer um das Datum und die Uhrzeit anzuzeigen
}


MainAppPrintCheck::~MainAppPrintCheck()
{
	if (m_SimulationPLC)
	{
		m_SimulationPLC->m_Terminate = true;
		m_SimulationPLC->wait();
	}
}


void MainAppPrintCheck::timerEvent(QTimerEvent *event)
{
	if(GetMainGUIPrintCheck())
	   GetMainGUIPrintCheck()->ShowCurrentDateTime(QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
}


void MainAppPrintCheck::ShowVideoPlayerButtons(bool show, int CameraIndex)
{
	if(GetMainGUIPrintCheck())
	   GetMainGUIPrintCheck()->ShowVideoPlayerButtons(show,CameraIndex);
}


void MainAppPrintCheck::SlotSetSliderValues(int MaxValue, int Number, int CameraIndex)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetSliderValues(MaxValue, Number, CameraIndex);
}


int  MainAppPrintCheck::GetCameraWidthInPixel(int index)
{
	int rv = 1024;
	if (GetBremsSchlauchKontrollePlugin())
	{
		rv = GetBremsSchlauchKontrollePlugin()->GetCameraWidthInPixel(index);
	}
	return rv;
}


int  MainAppPrintCheck::GetCameraHeightInPixel(int index)
{
	int rv = 1024;
	if (GetBremsSchlauchKontrollePlugin())
	{
		rv = GetBremsSchlauchKontrollePlugin()->GetCameraHeightInPixel(index);
	}
	return rv;
}


int  MainAppPrintCheck::GetBinningHorizontal(int index)
{
	int rv = 1;
	if (GetBremsSchlauchKontrollePlugin())
	{
		rv = GetBremsSchlauchKontrollePlugin()->GetBinningHorizontal(index);
	}
	return rv;
}


int  MainAppPrintCheck::GetBinningVertical(int index)
{
	int rv = 1;
	if (GetBremsSchlauchKontrollePlugin())
	{
		rv = GetBremsSchlauchKontrollePlugin()->GetBinningVertical(index);
	}
	return rv;
}


int  MainAppPrintCheck::GetAcquisitionLineRate(int index)
{
	int rv = 7000;
	if (GetBremsSchlauchKontrollePlugin())
	{
		rv = GetBremsSchlauchKontrollePlugin()->GetAcquisitionLineRate(index);
	}
	return rv;
}


int  MainAppPrintCheck::GetExposureTime(int index)
{
	int rv = 100;
	if (GetBremsSchlauchKontrollePlugin())
	{
		rv = GetBremsSchlauchKontrollePlugin()->GetExposureTime(index);
	}
	return rv;
}


void MainAppPrintCheck::SetCameraDeviceWidthInPixel(int set, int index)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetCameraDeviceWidthInPixel(set, index);
	}
}


void MainAppPrintCheck::SetCameraDeviceHeightInPixel(int set, int index)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetCameraDeviceHeightInPixel(set, index);
	}
}


void MainAppPrintCheck::SetBinningHorizontal(int set, int index)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetBinningHorizontal(set, index);
	}
}


void MainAppPrintCheck::SetBinningVertical(int set, int index)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetBinningVertical(set, index);
	}
}


void MainAppPrintCheck::SetAcquisitionLineRate(int set, int index)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetAcquisitionLineRate(set, index);
	}
}


void MainAppPrintCheck::SetExposureTime(int set, int index)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetExposureTime(set, index);
	}
}


/*void MainAppPrintCheck::SlotLoginBertram()
{
	SetLoginBertram();
}
*/


void MainAppPrintCheck::FinishedMeasuringAndImageAcquisition()
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		m_ImageDataSet.at(i)->WaitForFinshed();
	}
	for (int i = 0; i < m_ImageDataSet.count(); i++)
		delete m_ImageDataSet.at(i);
	m_ImageDataSet.clear();
	ClearProductList();
	if (m_SimulationPLC)
	{
		m_SimulationPLC->m_Terminate=true;
	}
	if (m_CheckIsDoWorkRunning)
	{
		m_CheckIsDoWorkRunning->m_Terminate = true;
	}
}


unsigned __int64 MainAppPrintCheck::GetCurrentTimeStamp()
{
	auto epoch = std::chrono::steady_clock::now().time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
	unsigned __int64   CurrentTimeStamp = value.count();
	return CurrentTimeStamp;
}


void MainAppPrintCheck::currentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState)
{
	if (m_PLC)
		m_PLC->setCurrentMachineState(machineState, diagState);
}


void MainAppPrintCheck::SlotMeasuringIsStopped(int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX && !m_InspectionIsStoppedCameraTop)
	{
		m_InspectionIsStoppedCameraTop = true;
		if (m_InspectionIsStoppedCameraBot && m_InspectionIsStoppedCameraTop)
		{
			InspectionIsActiveToPLC(false);//Antwort an die SPS Messung Kamera 1 und 2 ist gestopped
		}
	}
	if (CameraIndex == CAMERA_BOT_INDEX && !m_InspectionIsStoppedCameraBot)
	{
		m_InspectionIsStoppedCameraBot = true;
		if (m_InspectionIsStoppedCameraBot && m_InspectionIsStoppedCameraTop)
		{
			InspectionIsActiveToPLC(false);//Antwort an die SPS Messung Kamera 1 und 2 ist gestopped
		}
	}
}
//Funktionen von der SPS zum Kontrollsystem/Bildverarbeitung
//neuese Produkt laden
void MainAppPrintCheck::SlotLoadJob(int JobID)
{
		ProductData *pProductData = GetProductByProductID(JobID);
		if (pProductData)
		{
			ActivateProduct(pProductData->m_ProductName);
			qDebug() << "Load Product " << pProductData->m_ProductName << "From PLC";
		}
}

//Messung Starten oder Stoppen  (Produktion/Einrichten)
void MainAppPrintCheck::SlotInspectionActive(bool Active)
{
	QString Text;
	if (GetMainGUIPrintCheck())
	{
		if (Active)
		{
			GetMainGUIPrintCheck()->SetSignalStartMeasuring();
		}
		else
		{
			m_InspectionIsStoppedCameraBot = m_InspectionIsStoppedCameraTop = false;
			GetMainGUIPrintCheck()->SetSignalStopMeasuring();
		}
	}
	if (Active)
		Text = "Set Start Inspection From PLC";
	else
		Text = "Set Stop Inspection From PLC";
	qDebug() << Text;
}

//Rücksetzen Fehler
void MainAppPrintCheck::SlotResetFault()
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->reset();
	}
	qDebug() << "Set Reset Error";
}
//Funktionen von Kontrollsystem/Bildverarbeitung nach SPS  
//Bestätigung neuer Job ist gesetzt
void MainAppPrintCheck::AckNewJobIdIsSet(int id)
{
	if (m_PLC)
		m_PLC->SetAckJobID(id);
}
//Inspection aktiv. System befindet sich im Inspectionsmodus
//Bestätigung an die SPS Inspektion ist an oder aus
void MainAppPrintCheck::InspectionIsActiveToPLC(bool Active)
{
	if (m_PLC)
		m_PLC->SetAckInspectionActive(Active);
}


void MainAppPrintCheck::SlotDelayTupeEndReached()
{
	//int DelayTimeInms=
	//QTimer::singleShot(5000, this, SLOT(SlotTubeEndReached());
}


void MainAppPrintCheck::SlotTubeEndReached()
{
	bool MeasuringIsRunning = false;
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		m_ImageDataSet.at(i)->EventFromPLCTubeEndIsReached();
		if (m_ImageDataSet.at(i)->IsResumeMeasuring())
			MeasuringIsRunning = true;
	}
	if(MeasuringIsRunning)
	   IncrementNumberHose();
    qDebug() << "Tube End Reached";
}


void MainAppPrintCheck::SetMaxNumberOfImagesInDir(int set)
{
	m_MaxNumberOfImagesInDir = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("MaxNumberOfImagesInDir", QVariant(set));
}


void MainAppPrintCheck::SetMaxErrorsBehindEachOther(int set)
{
	m_MaxErrorsBehindEachOther = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("MaxErrorsBehindEachOther", QVariant(set));
}


void MainAppPrintCheck::SetIntervallBetweenTwoErrors(int set)
{
	m_IntervallBetweenTwoErrors = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("IntervallBetweenTwoErrors", QVariant(set));
}


void MainAppPrintCheck::SetCameraCenterOffsetInMM(double set)
{
	m_CameraCenterOffsetInMM = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("CameraCenterOffsetInMM", QVariant(set));
}


/*void MainAppPrintCheck::SetLoginBertram()
{
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetLoginBertram();
}
*/


void MainAppPrintCheck::SetTransmissionDistanceCameraTopPLCInMM(double set)
{
	m_TransmissionDistanceCameraTopPLCInMM = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("TransmissionDistanceCameraTopPLCInMM", QVariant(set));
}


void MainAppPrintCheck::SetTransmissionDistanceCameraBotPLCInMM(double set)
{
	m_TransmissionDistanceCameraBotPLCInMM = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("TransmissionDistanceCameraBotPLCInMM", QVariant(set));
}



void MainAppPrintCheck::SetTopCameraIsFirst(bool set)
{
	m_TopCameraIsFirst = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("TopCameraIsFirst", QVariant(set));
}


void MainAppPrintCheck::SetToggelTimeLiveAndMeasureViewInms(int set)
{
	m_ToggelTimeLiveAndMeasureViewInms = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("ToggelTimeLiveAndMeasureViewInms", QVariant(set));
}


void MainAppPrintCheck::SetSaveErrorImagePoolCondition(int set)
{
	m_SaveErrorImagePoolCondition = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("SaveErrorImagePoolCondition", QVariant(set));
}

//wert wird nicht gespeichert damit immer sichergestellt ist, dass beim Starten der Software der Wert immer auf false steht
void MainAppPrintCheck::SetCameraTopSimulationOn(bool set)
{
	m_SimulationCameraTopOn = set;
}

//wert wird nicht gespeichert
void MainAppPrintCheck::SetCameraBotSimulationOn(bool set)
{
	m_SimulationCameraBotOn = set;
}


void MainAppPrintCheck::SetSpeedSimulationInMPerMin(double set)
{
	m_SpeedSimulationInMPerMin=set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("SpeedSimulationInMPerMin", QVariant(set));
}


void MainAppPrintCheck::SetPixelSizeCameraTopInMMPerPixel(double set)
{
	m_PixelSizeCameraTopInMMPerPixel = set;
	if (GetBremsSchlauchKontrollePlugin())
	    GetBremsSchlauchKontrollePlugin()->SetPreference("PixelSizeCameraTopInMMPerPixel", QVariant(set));
}


void MainAppPrintCheck::SetPixelSizeCameraBotInMMPerPixel(double set)
{
	m_PixelSizeCameraBotInMMPerPixel = set;
	if(GetBremsSchlauchKontrollePlugin())
	   GetBremsSchlauchKontrollePlugin()->SetPreference("PixelSizeCameraBotInMMPerPixel", QVariant(set));
}


void MainAppPrintCheck::SetNumberFormatsInReferenceImageView(int set)
{
	m_NumberFormatsInReferenceImageView = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("", QVariant(set));
}


void MainAppPrintCheck::SetWaitTimeAfterTubEndCameraTopInms(int set)
{
	m_WaitTimeAfterTubEndCameraTopInms = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("WaitTimeAfterTubEndCameraTopInms", QVariant(set));
}


void MainAppPrintCheck::SetWaitTimeAfterTubEndCameraBotInms(int set)
{
	m_WaitTimeAfterTubEndCameraBotInms = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("WaitTimeAfterTubEndCameraBotInms", QVariant(set));
}


void MainAppPrintCheck::SetMaxNumberCameraTopImagesHoseNotFound(int set)
{
	m_MaxNumberCameraTopImagesHoseNotFound = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("MaxNumberCameraTopImagesHoseNotFound", QVariant(set));
}


void MainAppPrintCheck::SetMaxNumberCameraBotImagesHoseNotFound(int set)
{
	m_MaxNumberCameraBotImagesHoseNotFound = set;
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetPreference("MaxNumberCameraBotImagesHoseNotFound", QVariant(set));
}


void MainAppPrintCheck::SetCurrentMaschineState(PluginInterface::MachineState set)
{
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetCurrentMaschineState(set);
}


void MainAppPrintCheck::SetEnableShowProductWindow(bool set)
{
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->SetEnableShowProductWindow(set);
}


void MainAppPrintCheck::OpenProductDialog()
{
	if (GetPopupDialogProductDialog())
	{
		GetPopupDialogProductDialog()->show();
		GetPopupDialogProductDialog()->setWindowTitle(tr("Open Product"));
	}
}


ProductDialog *MainAppPrintCheck::GetProductDialog()
{
	if (GetPopupDialogProductDialog() && GetPopupDialogProductDialog()->GetProductDialog())
		return GetPopupDialogProductDialog()->GetProductDialog();
	else
		return NULL;
}


void MainAppPrintCheck::SetSignalInspectionErrorText(int CameraIndex, const QString &ErrorText,bool Error)
{
    if (CameraIndex == CAMERA_TOP_INDEX) {
        emit SignalShowInspectionErrorTextCamTop(ErrorText, Error);
        if (Error) {
            qWarning() << "Top:" << ErrorText;
        } else {
            qDebug() << "Top:" << ErrorText;
        }
    }
    else {
        emit SignalShowInspectionErrorTextCamBot(ErrorText, Error);
        if (Error) {
            qWarning() << "Bottom:" << ErrorText;
        }
        else {
            qDebug() << "Bottom:" << ErrorText;
        }
    }
}


void MainAppPrintCheck::SetCameraStatus(int CameraIndex, QString Text,bool Simulation)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetCameraStatus(CameraIndex, Text, Simulation);
}


void MainAppPrintCheck::SlotShowCalculatetPixelSize(double PixelSize, double Diameter, int CameraIndex)
{
	if (GetWidgetEditGeneralSettings() && GetWidgetEditGeneralSettings()->isVisible())
	{
		GetWidgetEditGeneralSettings()->ShowCalculatetPixelSize(PixelSize, Diameter, CameraIndex);
	}
}


void MainAppPrintCheck::ShowImageCounter(int CameraIndex, double set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->ShowImageCounter(CameraIndex, set);
}


void MainAppPrintCheck::ShowSelectedRectKoordinates(int CameraIndex, QRectF &rect)
{
	if (GetWidgetEditReferenceData())
	{
		GetWidgetEditReferenceData()->ShowSelectedRectKoordinates(CameraIndex,rect);
	}
}


void MainAppPrintCheck::SetInspectionTime(int CameraIndex,double set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetInspectionTime(CameraIndex,set);
}


void MainAppPrintCheck::SetMeanDefectScore(int CameraIndex,double set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetMeanDefectScore(CameraIndex,set);
}


void MainAppPrintCheck::SetMaxDefectScore(int CameraIndex, double set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetMaxDefectScore(CameraIndex, set);
}


void MainAppPrintCheck::SetCameraFramesPerSecond(int CameraIndex, double set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetCameraFramesPerSecond(CameraIndex, set);
}


void MainAppPrintCheck::SetMaxCenterOffset(int CameraIndex, double set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetMaxCenterOffset(CameraIndex, set);
}


bool MainAppPrintCheck::IsCheckedShowOnlyErrorImage(int CameraIndex)
{
	bool rv = false;
	if (GetMainGUIPrintCheck())
		rv=GetMainGUIPrintCheck()->IsCheckedShowOnlyErrorImage(CameraIndex);
	return rv;
}


void MainAppPrintCheck::HideWidgetMeasureIsRunning(bool set)
{
	if (GetBremsSchlauchKontrollePlugin())
		GetBremsSchlauchKontrollePlugin()->HideWidgetMeasureIsRunning(set);
}


void MainAppPrintCheck::SlotSetHoseStart(bool set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetHoseStart(set);
}


void MainAppPrintCheck::SlotSetHoseMiddle(bool set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetHoseMiddle(set);
}


void MainAppPrintCheck::SlotSetHoseEnd(bool set)
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->SetHoseEnd(set);
}


void MainAppPrintCheck::SlotCurrentSpeedFromPLC(double SpeedInMPerMin)
{
	double LineRateInHerz;
	int iLineRateInHerz;

	for (int index = 0; index < NUMBER_CAMERAS; index++)
	{
		if (GetImageData(index) && SpeedInMPerMin > 0.0)
		{
			LineRateInHerz=GetImageData(index)->GetCalculatetCameraAcquisitionLineRate(SpeedInMPerMin);//[1/sec]
			iLineRateInHerz = static_cast<int>(LineRateInHerz + 0.5);
			SetAcquisitionLineRate(iLineRateInHerz, GetImageData(index)->GetCameraIndex());
		}
	}
    qDebug() << QString("Speed Changed:%1 m/min").arg(SpeedInMPerMin, 0, 'f', 2);
}


void MainAppPrintCheck::IncrementNumberHose()
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->IncrementNumberHose();
}


void MainAppPrintCheck::IncrementNumberErrorHose()
{
	if (GetMainGUIPrintCheck())
		GetMainGUIPrintCheck()->IncrementNumberErrorHose();
}


void MainAppPrintCheck::SlotNewReferenceDataGeneratet(int CameraID ,const QString ErrorMsg)
{
	if (GetWidgetEditReferenceData())
	{
		if (CameraID == CAMERA_TOP_INDEX)
			GetWidgetEditReferenceData()->GetWidgetEditReferenceCameraTop()->ShowInfoBoxStatusGeneraterefData(ErrorMsg);
		else
			GetWidgetEditReferenceData()->GetWidgetEditReferenceCameraBot()->ShowInfoBoxStatusGeneraterefData(ErrorMsg);
	}
}


void MainAppPrintCheck::SlotAddNewMessage(const QString &ErrorMsg, QtMsgType MsgType)
{
	if (GetBremsSchlauchKontrollePlugin())
	    GetBremsSchlauchKontrollePlugin()->SetMessage(ErrorMsg, MsgType);

	if (MsgType == QtMsgType::QtFatalMsg && m_LastErrorLogMsg!= ErrorMsg)
	{
		m_LastErrorLogMsg = ErrorMsg;
		WriteLogFile(m_LastErrorLogMsg, QString("ErrorLog.txt"));
        qWarning() << ErrorMsg;
	}
}


void MainAppPrintCheck::SlotAddNewDebugInfo(const QString &ErrorMsg, int InfoCode)
{
	
}


void MainAppPrintCheck::StartupApplication()
{
	int rv = ERROR_CODE_NO_ERROR;
	QString ErrorMsg;

	rv = LoadAllProductFiles(ErrorMsg);
	if (rv != ERROR_CODE_NO_ERROR)
	{//no Products defined, create default product
		m_LastLoadedProduct = QString("DefaultType");
		rv = WriteAndInsertNewProduct(m_LastLoadedProduct, QString(""), ErrorMsg);
		if (rv != ERROR_CODE_NO_ERROR)
		{
			SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
		}
		ShowAndSetCurrentProductName(m_LastLoadedProduct);
	}
	else
		ShowAndSetCurrentProductName(m_LastLoadedProduct);
}


void  MainAppPrintCheck::ShowAndSetCurrentProductName(QString &Name)
{
	if (GetBremsSchlauchKontrollePlugin())
	{
		GetBremsSchlauchKontrollePlugin()->SetPreference("LastProduct", QVariant(Name));//speichern in registry
		GetBremsSchlauchKontrollePlugin()->SetCurrentProductName(Name);//anzeige an die GUI
	}
}


void  MainAppPrintCheck::ActivateProduct(QString &ProductName)
{
	m_LastLoadedProduct = ProductName;
	ShowAndSetCurrentProductName(ProductName);
	ProductData *pProduct= GetCurrentProductData();
	if (pProduct)
	{
		AckNewJobIdIsSet(pProduct->m_ProductID);//Bestätigung an die SPS Product ist gesetzt
	}
}


int MainAppPrintCheck::WriteAndInsertNewProduct(QString &ProductName, QString &CopyFromProductName, QString &ErrorMsg)
{
	ProductData *pProductData = new ProductData(this, ProductName);
	ProductData *pCopyProductData = GetProductByProductName(CopyFromProductName);
	int rv;
	bool next = false;
	QString SourceFilePath, TargetFilePath, CameraDirName;

	if (pCopyProductData)
	{
		for (int camIndex = 0; camIndex < NUMBER_CAMERAS; camIndex++)
		{
			if (camIndex == CAMERA_TOP_INDEX)
				CameraDirName = CAMERA_TOP_DIR_NAME;
			else
				CameraDirName = CAMERA_BOT_DIR_NAME;
			SourceFilePath = GetProductLocation() + QString("/") + CopyFromProductName + QString("/") + CameraDirName;
			TargetFilePath = GetProductLocation() + QString("/") + ProductName         + QString("/") + CameraDirName;
			CopyRecursively(SourceFilePath, TargetFilePath);
			
		}
		*pProductData               = *pCopyProductData;
		pProductData->m_ProductName = ProductName;
	}
	pProductData->m_ProductID = GenerateNewProductID();//jobID
	rv = pProductData->WriteProductData(ErrorMsg);
	if (pCopyProductData)
	   rv = pProductData->ReadProductData(ErrorMsg);//damit die ModelID neu geladen wird
	if (rv == ERROR_CODE_NO_ERROR)
		m_ListProducts.insert(0, pProductData);
	else
		delete pProductData;
	return rv;
}


bool MainAppPrintCheck::CopyRecursively(QString sourceFolder, QString destFolder)
{
	bool success = false;
	QDir sourceDir(sourceFolder);

	if (!sourceDir.exists())
		return false;

	QDir destDir(destFolder);
	if (!destDir.exists())
		destDir.mkdir(destFolder);

	QStringList files = sourceDir.entryList(QDir::Files);
	for (int i = 0; i < files.count(); i++)
	{
		QString srcName  = sourceFolder + QDir::separator() + files[i];
		QString destName = destFolder   + QDir::separator() + files[i];
		success = QFile::copy(srcName, destName);
		if (!success)
			return false;
	}

	files.clear();
	files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < files.count(); i++)
	{
		QString srcName = sourceFolder + QDir::separator() + files[i];
		QString destName = destFolder  + QDir::separator() + files[i];
		success = CopyRecursively(srcName, destName);
		if (!success)
			return false;
	}

	return true;
}


int MainAppPrintCheck::RenameAndActivateProduct(QString &OldName, QString &NewName, QString &ErrorMsg)
{
	int retVal = ERROR_CODE_NO_ERROR;
	QFileInfoList FileInfolist = GetProductInfoList();
	bool NewNameExist = false;

	for (int i = 0; i < FileInfolist.count(); i++)
	{
		if (FileInfolist.at(i).baseName() == NewName)
		{
			NewNameExist = true;
			break;
		}
	}
	if (!NewNameExist)
	{
		QString PathAndFileName    = GetProductLocation() + QString("/") + OldName + QString("/") + OldName + QString(".dat");
		QString PathAndFileNewName = GetProductLocation() + QString("/") + OldName + QString("/") + NewName + QString(".dat");
		QFile RenamedFile(PathAndFileName);

		QDir RenameDir;

		RenamedFile.rename(PathAndFileNewName);

		if (OldName == GetCurrentProductName())
		{
			ShowAndSetCurrentProductName(NewName);
		}
		QString OldNameFolder = GetProductLocation() + QString("/") + OldName;
		QString NewNameFolder = GetProductLocation() + QString("/") + NewName;
		RenameDir.rename(OldNameFolder, NewNameFolder);

		ProductData *pProduct=GetProductByProductName(OldName);
		if (pProduct)
		{
			pProduct->SetNewProductLocationAndName(NewName);
			pProduct->WriteProductData(ErrorMsg);
		}
		if (GetPopupDialogProductDialog()->GetProductDialog())
			GetPopupDialogProductDialog()->GetProductDialog()->UpdateProductList();
	}
	else
	{
		ErrorMsg = tr("Can Not Rename Product. Name Exist! :%1").arg(NewName);
		retVal   = ERROR_CODE_ANY_ERROR;
	}
	return retVal;
}


ProductData *MainAppPrintCheck::GetProductByProductName(QString &Name)
{
	for (int i = 0; i < m_ListProducts.count(); i++)
	{
		if (m_ListProducts.at(i)->GetProductName() == Name)
		{
			return m_ListProducts.at(i);
		}
	}
	return NULL;
}


ProductData *MainAppPrintCheck::GetProductByProductID(int ID)
{
	for (int i = 0; i < m_ListProducts.count(); i++)
	{
		if (m_ListProducts.at(i)->m_ProductID == ID)
		{
			return m_ListProducts.at(i);
		}
	}
	return NULL;
}


bool  MainAppPrintCheck::ExistProduct(QString &ProductName)
{
	bool rv = false;

	for (int i = 0; i < m_ListProducts.count(); i++)
	{
		if (m_ListProducts.at(i)->GetProductName() == ProductName)
		{
			rv = true;
			break;
		}
	}
	return rv;
}


void MainAppPrintCheck::RemoveProduct(QString &ProductName)
{
	QString ErrorMsg, PathAndFileName = GetProductLocation() + QString("/") + ProductName;
	QDir DeletetPath;
	int rv = ERROR_CODE_NO_ERROR;

	DeletetPath.setPath(PathAndFileName);
	DeletetPath.removeRecursively();
	for (int i = 0; i < m_ListProducts.count(); i++)
	{
		if (m_ListProducts.at(i)->GetProductName() == ProductName)
		{
			delete m_ListProducts.at(i);
			m_ListProducts.removeAt(i);
			break;
		}
	}
}


QFileInfoList MainAppPrintCheck::GetProductInfoList()
{
	QDir Path(m_ProductLocation);
	
	return Path.entryInfoList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
}


void MainAppPrintCheck::ClearProductList()
{
	for (int i = 0; i < m_ListProducts.count(); i++)
		delete m_ListProducts.at(i);
	m_ListProducts.clear();
}


int MainAppPrintCheck::LoadAllProductFiles(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	QString BaseName,AbsolutePath,FileName;
	QFileInfoList list = GetProductInfoList();
	QFile ProductFile;
	bool ListIncludesCurrentProduct = false;


	ClearProductList();
	for (int i = 0; i < list.count(); i++)
	{
		BaseName = list.at(i).baseName();
		AbsolutePath = list.at(i).absolutePath();
		FileName = AbsolutePath + QString("/") + BaseName + QString("/") + BaseName + ".dat";
		ProductFile.setFileName(FileName);
		if(ProductFile.exists())
		   rv = ReadAndAppendProduct(BaseName, ErrorMsg);
	}
	if (list.count() == 0 || rv==ERROR_CODE_ANY_ERROR)
	{
		ErrorMsg = tr("No Products Defined");
		rv = ERROR_CODE_ANY_ERROR;
	}
	else
	{
		QString CurrentProduct = GetCurrentProductName();
		for (int i = 0; i < list.count(); i++)
		{
			BaseName = list.at(i).baseName();
			if (CurrentProduct == BaseName)
				ListIncludesCurrentProduct = true;
		}
		if (!ListIncludesCurrentProduct)
		{
			CurrentProduct = list.at(0).baseName();
			ShowAndSetCurrentProductName(CurrentProduct);
		}
	}
	return rv;
}


int MainAppPrintCheck::ReadAndAppendProduct(QString &ProductName, QString &ErrorMsg)
{
	ProductData *pProductData = new ProductData(this, ProductName);
	int rv = pProductData->ReadProductData(ErrorMsg);

	if (rv == ERROR_CODE_NO_ERROR)
		m_ListProducts.append(pProductData);
	else
		delete pProductData;
	return rv;
}


ProductData *MainAppPrintCheck::GetCurrentProductData()
{
	for (int i = 0; i < m_ListProducts.count(); i++)
	{
			if (m_ListProducts.at(i)->GetProductName() == m_LastLoadedProduct)//GetSettingsData()->m_CurrentProductName)
			{
				return m_ListProducts.at(i);
			}
	}
	return NULL;
}


int MainAppPrintCheck::GetNumberEvaluationPeriodFormatNotFound()
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
		return pProductData->m_NumberEvaluationPeriodFormatNotFound;
	return 2;
}


void MainAppPrintCheck::SetNumberEvaluationPeriodFormatNotFound(int set)
{
	ProductData *pProductData = GetCurrentProductData();
	if (pProductData)
	{
		QString ErrorMsg;
		pProductData->m_NumberEvaluationPeriodFormatNotFound = set;
		pProductData->WriteProductData(ErrorMsg);
	}
}


int MainAppPrintCheck::GenerateNewProductID()
{
	int ID = 1;
	for (int i = 0; i < m_ListProducts.count(); i++)
	{
		if (m_ListProducts.at(i)->m_ProductID == ID)
		{
			ID++;
			i = -1;
		}
	}
	return ID;
}


bool MainAppPrintCheck::ExistProductID(int ID)
{
	bool rv = false;
	for (int i = 0; i < m_ListProducts.count(); i++)
	{
		if (m_ListProducts.at(i)->m_ProductID == ID)
		{
			rv = true;
			break;
		}
	}
	return rv;
}


LiveImageView *MainAppPrintCheck::GetFormatImageView(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i)->GetFormatImageView();
	}
	return NULL;
}


LiveImageView *MainAppPrintCheck::GetReferenceImageView(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i)->GetReferenceImageView();
	}
	return NULL;
}


ErrorImageView  *MainAppPrintCheck::GetErrorImageView(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i)->GetErrorImageView();
	}
	return NULL;
}


CameraImageView  *MainAppPrintCheck::GetCameraImageView(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i)->GetCameraImageView();
	}
	return NULL;
}


ImageData *MainAppPrintCheck::GetImageData(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i);
	}
	return NULL;
}


int MainAppPrintCheck::GetCameraWidth(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i)->GetCameraWidthInPixel();
	}
	return 1024;
}


int MainAppPrintCheck::GetCameraHeight(int CameraIndex)
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		if (m_ImageDataSet.at(i)->GetCameraIndex() == CameraIndex)
			return m_ImageDataSet.at(i)->GetCameraHeightInPixel();
	}
	return 1024;
}


void MainAppPrintCheck::StartImageAcquisition()
{
	for (int i = 0; i < m_ImageDataSet.count(); i++)
	{
		m_ImageDataSet.at(i)->StartInspection();
	}
}


void MainAppPrintCheck::SetImageCameraTop(const cv::Mat &Image, unsigned long FrameNumber,int FrameStatus)
{
	ImageData *pImageData=GetImageData(CAMERA_TOP_INDEX);
	if (pImageData)
		pImageData->GetMeasureTaskDetectHose()->NewIncommingImage(Image);
}


void MainAppPrintCheck::SetImageCameraBot(const cv::Mat &Image, unsigned long FrameNumber, int FrameStatus)
{
	ImageData *pImageData = GetImageData(CAMERA_BOT_INDEX);
	if (pImageData)
		pImageData->GetMeasureTaskDetectHose()->NewIncommingImage(Image);
}


int MainAppPrintCheck::WriteLogFile(const QString &data, const QString &FileName)
{
	int rv = ERROR_CODE_NO_ERROR;
	QString PathAndName = GetLogFileLocation() + QString("/") + FileName;
	QString  ErrorMsg;
	QFile file(PathAndName);
	QString Time = QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz");
	QString Date = QDateTime::currentDateTime().date().toString("dd.MM.yyyy");
	QFileInfo FileInfo(PathAndName);
	int maxFileSize = 1000000;


	if (FileInfo.size() > maxFileSize)
		file.remove();
	if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		QTextStream os(&file);
		os << Date << " " << Time << " " << data << "\r\n";
		file.close();
	}
	else
	{
		ErrorMsg = tr("Can Not Open Data File: %1").arg(PathAndName);
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


void MainAppPrintCheck::GetDisplayZoomedSizeFormatImage(int &DisplayWidth, int &DisplayHeight)
{
	DisplayWidth  = DisplayWidth  * GetDisplayZoomFactorFormatImage();
	DisplayHeight = DisplayHeight * GetDisplayZoomFactorFormatImage();
}


void MainAppPrintCheck::GetDisplayZoomedSizeCameraImage(int &DisplayWidth, int &DisplayHeight)
{
	DisplayWidth  = DisplayWidth  * GetDisplayZoomFactorCameraImage();
	DisplayHeight = DisplayHeight * GetDisplayZoomFactorCameraImage();
}




