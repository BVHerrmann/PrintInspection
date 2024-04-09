#pragma once
#include <QtCore>
#include "qwidget.h"
#include "GlobalConst.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "MainLogic.h"

#include "qlocalsocket.h"




class CheckIsDoWorkRunning : public QRunnable
{
public:
	CheckIsDoWorkRunning(MainLogic *pMainLogic)
	{
		m_MainLogic = pMainLogic;
		m_Terminate = false;
	};
	void run()
	{
		while (!m_Terminate)
		{
			m_MainLogic->IsDoWorkRunning();

		}
	}
public:
	MainLogic *m_MainLogic;
	bool m_Terminate;
};


class SimulationPLC : public QThread
{
	Q_OBJECT
public:
	SimulationPLC(MainLogic *pMainLogic)
	{
		m_MainLogic = pMainLogic;
		m_Terminate = false;
		m_Name = "SPSSimu";
		m_Client = new QLocalSocket();
		
	};
	void run()
	{
		bool rv;
		while (!m_Terminate)
		{
			if(m_Client->state() != QLocalSocket::ConnectedState)
			    m_Client->connectToServer(m_Name);
			else
			{
				m_MainLogic->SetCurrentLineSpeed(30.0);
				rv = m_MainLogic->SetCurrentMeasuringResults();
				if (rv)
				{
					QString data = QString("Err");
					m_Client->write(data.toLatin1());
					m_Client->flush();
				}
			}
			msleep(10);
		}
	}
public:
	MainLogic *m_MainLogic;
	bool m_Terminate;
	QLocalSocket *m_Client;
	QString m_Name;
};

class WidgetEditMeasureParameter;
class BremsSchlauchKontrollePlugin;
class ImageData;
class LiveImageView;
class ErrorImageView;
class CameraImageView;
class ProductData;
class MainGUIPrintCheck;
class WidgetEditReferenceData;
class WidgetEditProductData;
class ProductDialog;
class PopupDialogProductDialog;
class WidgetEditCustomSettings;
class WidgetEditGeneralSettings;
class MainAppPrintCheck : public QObject
{
	Q_OBJECT
public:
	MainAppPrintCheck(BremsSchlauchKontrollePlugin *);
	virtual ~MainAppPrintCheck();


	BremsSchlauchKontrollePlugin *GetBremsSchlauchKontrollePlugin() { return m_BremsSchlauchKontrollePlugin; }
	MainGUIPrintCheck            *GetMainGUIPrintCheck()            { return m_MainGUIPrintCheck; }
	WidgetEditReferenceData      *GetWidgetEditReferenceData()      { return m_WidgetEditReferenceData; }
	WidgetEditProductData        *GetWidgetEditProductData()        { return m_WidgetEditProductData; }
	WidgetEditCustomSettings     *GetWidgetEditCustomSettings()     { return m_WidgetEditCustomSettings;}
	WidgetEditGeneralSettings    *GetWidgetEditGeneralSettings()    { return m_WidgetEditGeneralSettings; }
	WidgetEditMeasureParameter   *GetWidgetEditMeasureParameterCameraTop() { return m_WidgetEditMeasureParameterCameraTop; }
	WidgetEditMeasureParameter   *GetWidgetEditMeasureParameterCameraBot() { return m_WidgetEditMeasureParameterCameraBot; }
	ImageData                    *GetImageData(int CameraIndex);
	LiveImageView                *GetFormatImageView(int CameraIndex);
	ErrorImageView               *GetErrorImageView(int CameraIndex);
	LiveImageView                *GetReferenceImageView(int CameraIndex);
	CameraImageView              *GetCameraImageView(int CameraIndex);
	void                          StartImageAcquisition();
	void                          FinishedMeasuringAndImageAcquisition();
	int                           WriteLogFile(const QString &data, const QString &FileName);
	QString GetLogFileLocation() { return m_LogFileLocation; }
	QString GetProductLocation() { return m_ProductLocation; }
	QString GetVideoFileLocationCameraTop() { return m_VideoFileLocationCameraTop; }
	QString GetVideoFileLocationCameraBot() { return m_VideoFileLocationCameraBot; }
	QString GetResultsLocationCameraBot()   { return m_ResultsLocationCameraBot; }
	QString GetResultsLocationCameraTop()   { return m_ResultsLocationCameraTop; }

	int                       LoadAllProductFiles(QString &ErrorMsg);
	QFileInfoList GetProductInfoList();
	void StartupApplication();
	void ClearProductList();
	int  WriteAndInsertNewProduct(QString &ProductName, QString &CopyFromProductName, QString &ErrorMsg);
	ProductData *GetProductByProductName(QString &Name);
	ProductData *GetProductByProductID(int ID);
	ProductData *GetCurrentProductData();
	int ReadAndAppendProduct(QString &ProductName, QString &ErrorMsg);
	void ShowAndSetCurrentProductName(QString &Name);
	QString  GetCurrentProductName() { return m_LastLoadedProduct; }

	int GetToggelTimeLiveAndMeasureViewInms() { return m_ToggelTimeLiveAndMeasureViewInms; }
	void SetToggelTimeLiveAndMeasureViewInms(int set);
	int GetNumberFormatsInReferenceImageView() { return m_NumberFormatsInReferenceImageView; }
	int GetSaveErrorImagePoolCondition() { return m_SaveErrorImagePoolCondition; }
	void SetSaveErrorImagePoolCondition(int set);
	double GetPixelSizeCameraTopInMMPerPixel() { return m_PixelSizeCameraTopInMMPerPixel; }
	double GetPixelSizeCameraBotInMMPerPixel() { return m_PixelSizeCameraBotInMMPerPixel; }

	void SetPixelSizeCameraTopInMMPerPixel(double set);
	void SetPixelSizeCameraBotInMMPerPixel(double set);

	double GetDisplayZoomFactorFormatImage() { return m_DisplayZoomFactorFormatImage; }
	double GetDisplayZoomFactorCameraImage() { return m_DisplayZoomFactorCameraImage; }

	void SetNumberFormatsInReferenceImageView(int set);
	void SetTopCameraIsFirst(bool set);
	bool GetTopCameraIsFirst() { return m_TopCameraIsFirst; }


	void GetDisplayZoomedSizeFormatImage(int &DisplayWidth, int &DisplayHeight);
	void GetDisplayZoomedSizeCameraImage(int &DisplayWidth, int &DisplayHeight);
	void HideWidgetMeasureIsRunning(bool set);

	void IncrementNumberHose();
	void IncrementNumberErrorHose();
	void SetInspectionTime(int CameraIndex, double set);
	void SetMeanDefectScore(int CameraIndex, double set);
	void SetMaxDefectScore(int CameraIndex, double set);
	void SetCameraFramesPerSecond(int CameraIndex, double set);
	void SetMaxCenterOffset(int CameraIndex, double set);
	void ShowImageCounter(int CameraIndex, double set);
	bool IsCheckedShowOnlyErrorImage(int CameraIndex);
	void ShowSelectedRectKoordinates(int CameraIndex, QRectF &rect);
	int GenerateNewProductID();
	bool ExistProductID(int ID);

	void SetImageCameraTop(const cv::Mat &Image, unsigned long FrameNumber, int FrameStatus);
	void SetImageCameraBot(const cv::Mat &Image, unsigned long FrameNumber, int FrameStatus);

	QList<ProductData*>        *GetListProducts() { return &m_ListProducts; }
	PopupDialogProductDialog *GetPopupDialogProductDialog() { return m_PopupDialogProductDialog; }
	ProductDialog *GetProductDialog();

	bool  ExistProduct(QString &ProductName);
	void  RemoveProduct(QString &ProductName);
	void  ActivateProduct(QString &ProductName);

	void OpenProductDialog();
	bool CopyRecursively(QString sourceFolder, QString destFolder);
	void SetEnableShowProductWindow(bool set);

	int RenameAndActivateProduct(QString &OldName, QString &NewName, QString &ErrorMsg);

	bool IsSimulationCameraTopOn() { return m_SimulationCameraTopOn; }
	bool IsSimulationCameraBotOn() { return m_SimulationCameraBotOn; }

	void SetCameraTopSimulationOn(bool set);
	void SetCameraBotSimulationOn(bool set);

	void SetSpeedSimulationInMPerMin(double set);
	double GetSpeedSimulationInMPerMin() { return m_SpeedSimulationInMPerMin; }

	double GetCameraCenterOffsetInMM()            { return m_CameraCenterOffsetInMM; }
	double GetTransmissionDistanceCameraTopPLCInMM() {	return m_TransmissionDistanceCameraTopPLCInMM;	}
	double GetTransmissionDistanceCameraBotPLCInMM() {  return m_TransmissionDistanceCameraBotPLCInMM; }

	void SetCameraCenterOffsetInMM(double set);
	void SetTransmissionDistanceCameraTopPLCInMM(double set);
	void SetTransmissionDistanceCameraBotPLCInMM(double set);

	
	void InspectionIsActiveToPLC(bool Active);
	void AckNewJobIdIsSet(int id);
	void SetCameraStatus(int CameraIndex, QString Text,bool Simulation);

	MainLogic *GetPLC() {return m_PLC.get();}
	std::shared_ptr<MainLogic> GetPLCSharedPtr() { return m_PLC; }
	void currentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState);

	void SetSignalInspectionErrorText(int CameraIndex, const QString &ErrorText,bool Error);

	int GetCameraWidth(int CameraIndex);
	int GetCameraHeight(int CameraIndex);

	int  GetNumberEvaluationPeriodFormatNotFound();
	void SetNumberEvaluationPeriodFormatNotFound(int set);

	unsigned __int64 GetCurrentTimeStamp();

	void SetCurrentMaschineState(PluginInterface::MachineState set);

	//void SetLoginBertram();
	void ShowVideoPlayerButtons(bool show, int CameraIndex);

	double GetMaxHoseLenghtInMM() {	return m_MaxHoseLenghtInMM;	}

	int GetWaitTimeAfterTubEndCameraTopInms() { return m_WaitTimeAfterTubEndCameraTopInms; }
	int GetWaitTimeAfterTubEndCameraBotInms() { return m_WaitTimeAfterTubEndCameraBotInms; }
	int GetMaxNumberCameraTopImagesHoseNotFound() { return m_MaxNumberCameraTopImagesHoseNotFound; }
	int GetMaxNumberCameraBotImagesHoseNotFound() { return m_MaxNumberCameraBotImagesHoseNotFound; }

	void SetWaitTimeAfterTubEndCameraTopInms(int set);
	void SetWaitTimeAfterTubEndCameraBotInms(int set);
	void SetMaxNumberCameraTopImagesHoseNotFound(int set);
	void SetMaxNumberCameraBotImagesHoseNotFound(int set);

	int GetMaxErrorsBehindEachOther()  { return m_MaxErrorsBehindEachOther; }
	int GetIntervallBetweenTwoErrors() { return m_IntervallBetweenTwoErrors; }

    bool   DisableDebugInfoMeasureResults() { return m_DisableShowDebugInfoMeasureResults; }
    bool   ShowDebugInfoOnlyError() { return m_ShowDebugInfoOnlyError; }

	void SetMaxErrorsBehindEachOther(int set);
	void SetIntervallBetweenTwoErrors(int set);
	
	int  GetCameraWidthInPixel(int index);
	int  GetCameraHeightInPixel(int index);
	int  GetBinningHorizontal(int index);
	int  GetBinningVertical(int index);
	int  GetAcquisitionLineRate(int index);
	int  GetExposureTime(int index);

	void SetCameraDeviceWidthInPixel(int set, int index);
	void SetCameraDeviceHeightInPixel(int set, int index);
	void SetBinningHorizontal(int set, int index);
	void SetBinningVertical(int set, int index);
	void SetAcquisitionLineRate(int set, int index);
	void SetExposureTime(int set, int index);

	void SetInspectionIsStoppedCameraTop(bool set) { m_InspectionIsStoppedCameraTop=set; }
	void SetInspectionIsStoppedCameraBot(bool set) { m_InspectionIsStoppedCameraBot=set; }

	double GetDefaultBlockWidthInMM()  { return m_DefaultBlockWidthInMM; }
	double GetDefaultBlockHeightInMM() { return m_DefaultBlockHeightInMM; }

	int GetMaxNumberOfImagesInDir() {return m_MaxNumberOfImagesInDir;}
	void SetMaxNumberOfImagesInDir(int set);// { return m_MaxNumberOfImagesInDir; }

	double GetMaxFormatLenghtInMM() { return m_MaxFormatLenghtInMM; }

	bool IsPLCSimulationOn() { return m_PLCSimulationOn; }

	bool    IsProgramHasAlreadyEnded() { return m_ProgramHasAlreadyEnded; }
	void    SetProgramHasAlreadyEnded(bool set) { m_ProgramHasAlreadyEnded=set; }

	bool RestartApplicationWhenCameraTimeout() {return m_RestartApplicationWhenCameraTimeout;}

	protected:
		void timerEvent(QTimerEvent *event);

public slots:
	void SlotAddNewMessage(const QString &ErrorMsg, QtMsgType MsgType);
	void SlotAddNewDebugInfo(const QString &ErrorMsg, int InfoCode);
	void SlotNewReferenceDataGeneratet(int CameraIndex, const QString ErrorMsg);
	void SlotSetHoseStart(bool set);
	void SlotSetHoseMiddle(bool set);
	void SlotSetHoseEnd(bool set);
	void SlotLoadJob(int JobID);
	void SlotInspectionActive(bool Active);
	void SlotResetFault();
	//void SlotLoginBertram();
	void SlotShowCalculatetPixelSize(double PixelSize,double Diameter, int CameraIndex);
	void SlotCurrentSpeedFromPLC(double Speed);
	void SlotSetSliderValues(int MaxValue, int Number, int CameraIndex);
	void SlotTubeEndReached();
	void SlotDelayTupeEndReached();
	void SlotMeasuringIsStopped(int CameraIndex);


signals:
	void SignalSetHoseStart(bool set);
	void SignalSetHoseMiddle(bool set);
	void SignalSetHoseEnd(bool set);
	void SignalShowInspectionErrorTextCamTop(const QString &ErrorText,bool Error);
	void SignalShowInspectionErrorTextCamBot(const QString &ErrorText, bool Error);


private:
	std::shared_ptr<MainLogic>    m_PLC;
	BremsSchlauchKontrollePlugin *m_BremsSchlauchKontrollePlugin;
	MainGUIPrintCheck            *m_MainGUIPrintCheck;
	WidgetEditReferenceData      *m_WidgetEditReferenceData;
	WidgetEditProductData        *m_WidgetEditProductData;
	WidgetEditCustomSettings     *m_WidgetEditCustomSettings;
	WidgetEditGeneralSettings    *m_WidgetEditGeneralSettings;
	WidgetEditMeasureParameter   *m_WidgetEditMeasureParameterCameraTop;
	WidgetEditMeasureParameter   *m_WidgetEditMeasureParameterCameraBot;
	PopupDialogProductDialog     *m_PopupDialogProductDialog;
	QString                       m_LogFileLocation;
	QString                       m_ProductLocation;
	QString                       m_LastLoadedProduct;
	QString                       m_VideoFileLocationCameraBot;
	QString                       m_VideoFileLocationCameraTop;
	QString                       m_ResultsLocationCameraBot;
	QString                       m_ResultsLocationCameraTop;
	QString                       m_ImageLocation;
	QString                       m_LastErrorLogMsg;
	QList<ImageData   *>          m_ImageDataSet;
	QList<ProductData *>          m_ListProducts;
	int                           m_ToggelTimeLiveAndMeasureViewInms;
	int                           m_NumberFormatsInReferenceImageView;
	int                           m_SaveErrorImagePoolCondition;
	int                           m_WaitTimeAfterTubEndCameraTopInms;
	int                           m_WaitTimeAfterTubEndCameraBotInms;
	int                           m_MaxNumberCameraTopImagesHoseNotFound;
	int                           m_MaxNumberCameraBotImagesHoseNotFound;
	int                           m_MaxErrorsBehindEachOther;
	int                           m_IntervallBetweenTwoErrors;
	int                           m_MaxNumberOfImagesInDir;
	bool                          m_SimulationCameraTopOn;
	bool                          m_SimulationCameraBotOn;
	bool                          m_InspectionIsStoppedCameraTop;
	bool                          m_InspectionIsStoppedCameraBot;
	bool                          m_TopCameraIsFirst;
	bool                          m_PLCSimulationOn;
    bool                          m_DisableShowDebugInfoMeasureResults;
    bool                          m_ShowDebugInfoOnlyError;
	bool                          m_ProgramHasAlreadyEnded;
	bool                          m_RestartApplicationWhenCameraTimeout;
	double                        m_DisplayZoomFactorFormatImage;
	double                        m_DisplayZoomFactorCameraImage;
	double                        m_PixelSizeCameraTopInMMPerPixel;
	double                        m_PixelSizeCameraBotInMMPerPixel;
	double                        m_CameraCenterOffsetInMM;
	double                        m_TransmissionDistanceCameraTopPLCInMM;
	double                        m_TransmissionDistanceCameraBotPLCInMM;
	double                        m_SpeedSimulationInMPerMin;
	double                        m_MaxHoseLenghtInMM;
	double                        m_DefaultBlockWidthInMM, m_DefaultBlockHeightInMM;
	double                        m_MaxFormatLenghtInMM;
	CheckIsDoWorkRunning         *m_CheckIsDoWorkRunning;
	SimulationPLC                *m_SimulationPLC;
};