#pragma once

#include <QtCore>
#include "qlistwidget.h"
#include "qtabwidget.h"
#include "ImageMetaData.h"
#include "ImageLineInformation.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"


typedef QHash <int, InspectionWindow>  ListResultsInspectionWindows;
typedef QHash <int, ListResultsInspectionWindows> ListResultsSubFormat;


class MainAppPrintCheck;//Parent Class
class LiveImageView;
class MeasureTaskDetectHose;
class MeasureTaskPrintCheck;
class MeasureTaskFormatCheck;
class ProductData;
class ErrorImageView;
class InspectionWindow;
class CameraImageView;
class SubFormatData;
class TableWidgetSubFormatResults;
class MatchingShapeBased;
class WidgetDetailResults;
class CameraSimulation;
class PrintLineDetector;
class ImageData : public QThread
{
	Q_OBJECT
public:
	ImageData(MainAppPrintCheck *pParent, int CameraID);
	~ImageData();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	virtual void   run();
	int GetCameraIndex() { return m_CameraIndex; }
	void WaitForFinshed();
	void NewIncommingImage(const cv::Mat &image);
	void StartInspection();
	void  CopyROIImage(cv::Rect &ROIRect, cv::Mat &Source, cv::Mat &ROIImage);

	void SetTabWidgetLiveOrErrorImage(int TabIndex);
	void GetDisplayZoomedSizeFormatImage(int &DisplayWidth, int &DisplayHeight);
	void GetDisplayZoomedSizeCameraImage(int &DisplayWidth, int &DisplayHeight);

	int GetToggelTimeLiveAndMeasureViewInms();
	double  GetDisplayZoomFactorFormatImage();
	double  GetDisplayZoomFactorCameraImage();

	LiveImageView  *GetFormatImageView() { return m_FormatImageView; }
	ErrorImageView *GetErrorImageView() { return m_ErrorImageView; }
	LiveImageView  *GetReferenceImageView() { return m_ReferenceImageView; }
	CameraImageView    *GetCameraImageView() { return  m_CameraImageView; }

	MatchingShapeBased  *GetMatchingShapeBased() { return m_MatchingShapeBased; }
	MatchingShapeBased  *GetMatchingFormatWindow() { return m_MatchingFormatWindow; }
	PrintLineDetector   *GetPrintLineDetector() { return m_PrintLineDetector; }

	WidgetDetailResults  *GetWidgetDetailResults() { return m_WidgetDetailResults; }
	CameraSimulation     *GetCameraSimulation()    { return m_CameraSimulation; }

	double GetPixelSize();
	void   SetPixelSize(double set);

	int  WriteLogFile(const QString &data, const QString &FileName);

	MeasureTaskDetectHose *GetMeasureTaskDetectHose() { return m_MeasureTaskDetectHose; }
	MeasureTaskPrintCheck *GetMeasureTaskPrintCheck() { return m_MeasureTaskPrintCheck; }
	MeasureTaskFormatCheck *GetMeasureTaskFormatCheck() { return m_MeasureTaskFormatCheck; }
	   
	bool IsResumeMeasuring() { return m_ResumeMeasuring; }
	void ResumeMeasuring(bool set) { m_ResumeMeasuring = set; }

	bool GetCheckNewReference() { return m_CheckNewReference; }
	void SetCheckNewReference(bool set) { m_CheckNewReference = set; }

	void SetEventGenerateReferenceData();

	ProductData *GetCurrentProductData();

	QRectF GetInspectionRectByIndex(int InsepectionWindowIndex);
	void   SetInspectionRectByIndex(QRectF &NewRect, int InsepectionWindowIndex);

	QRectF GetInspectionRect(int InsepectionWindowID);
	void   SetInspectionRect(QRectF &NewRect, int InsepectionWindowID);

	InspectionWindow *GetInspectionWindowByID(int InsepectionWindowID);
	InspectionWindow *GetInspectionWindowByIndex(int InsepectionWindowIndex);

	QString GetReferenceLocation();
	void WriteProductData();
	void AddNewInspectionRect(InspectionWindow *pInspectionWindow);
	int GetNumberInspectionWindows();
	void DeleteSelectedInspectionWindow();
	void RemoveInspectionRect(int InspectionID);
	void LoadAndShowReferenceImageFromDisk();

	SubFormatData *GetSubFormatData();
	void ShowMeanDefectScore(double Value);
	void ShowMaxDefectScore(double Value);
	
	void SetMaxCenterOffset(double set);
	void ShowImageCounter(double value);

	bool IsCheckedShowOnlyErrorImage();
	void RemoveNotValidInspectionWindow();
	void ShowSelectedRectKoordinates(QRectF &rect);

	int GetCameraWidthInPixel()  { return m_CameraWidthInPixel; }
	int GetCameraHeightInPixel() { return m_CameraHeightInPixel; }

	void SetCameraWidthInPixel(int set) { m_CameraWidthInPixel = set; }
	void SetCameraHeightInPixel(int set) { m_CameraHeightInPixel = set; }
	int GetNumberFormatsInReferenceImageView();
	void SetNumberFormatsInReferenceImageView(int set);
	double GetHoseDiameterInPixel();
	double GetHoseDiameterInMM();
	//double GetPrintErrorTolInPercent();
	double GetPositionTolInMM();
	double GetFormatLenghtTolInMM();
	double GetBlockHeightTolInMM();
	double GetBlockWidthTolInMM();

	void LoadAllMeasuringSettings();
	QString GetLocationProductData();

	void AppendInspectionResultsForResultView(const ImageMetaData &Image);
	
	ListResultsSubFormat *GetListResultsSubFormat() { return &m_ListResultsSubFormat; }
	ListResultsInspectionWindows *GetListResultsInspectionWindows() { return &m_ListResultsInspectionWindows; }

	void ShowDetailResults();
	void CalculateFormatRect();
	HalconCpp::HImage  *GetHalconRGBResultImage();
	double GetFormatLenghtInPixel();
	InspectionWindow  *GetInspectionWindowHoseDetection();
	void GetListMeasurementResults(InspectionWindow *pInspectionWindow, QStringList &List);
	QStringList  GetHeaderListInspectionResults() {return m_HeaderListInspectionResults;}

	void SaveInspectionResults(ImageMetaData &ResultAndImage);
	void AppendInspectionResultsForStorage(ImageMetaData &ResultAndImage);
	int GetSaveErrorImagePoolCondition();
	int WriteInspectionResults(QString &FileName, QString &data, bool WithDateAndTime);
	void SetEnableWriteFullHose(bool set);
	bool GetEnableWriteFullHose();
	double GetFullHoseLenghtInPixel();
	void StartTimerMeasureFullInspectionTime();
	void StopTimerMeasureFullInspectionTime();

	unsigned long  GetFormatCounter() {return m_FormatCounter;}
	void IncrementFormatCounter() { m_FormatCounter++; }
	void ResetFormatCounter() { m_FormatCounter=0; }

	int GetNumberHose();

	//QString GetCameraName();
	QImage CopyHalconImageIntoQtImage(HalconCpp::HImage  &HalconImage, int NumberChannelsQtImage);

	QVector<QRgb> GetColorTable() { return m_Colors; }
	
	bool GetTopCameraIsFirst();

	void StartCameraSimulation(bool Start);

	void GenerateImageTimeStampList(QList< ImageLineInformation> &ListLineInformation, int NumberLines, unsigned __int64   TimeStampInMuSec);
	void CalculateRectPosRelatetToFormatRect(SubFormatData *pSubFormatData, QRectF &FormatRect);

	void ShowInspectionErrorText(const QString &ErrorText,bool Error);

	void CheckImageDir(QString &Dir);

	QString GetResultLocation();

	int GetSimulationCameraIntervalInms();
	double GetCalculatetCameraAcquisitionLineRate(double SpeedInmPerMin);

	void ClearSharedMemory();
	void ResetFormatCheckData();

	void EventFromPLCTubeEndIsReached();
	void SetAdditionalResultData(QString &data);

	double  GetFullInspectionTimeInms() { return m_FullInspectionTimeInms; }

	void DetachSharedMemoryInPrintchecktask();

	double GetMaxFormatLenghtInPixel();

	void CreateSharedmemoryFullHose();
	

signals:
	void SignalShowMessage(const QString &ErrorMsg, int error_code);
	void SignalShowLiveImage(const ImageMetaData &Image);
	void SignalShowInspectionTime(double FullInspectionTimeInms);
	void SignalStartToggelTimerShowLiveView();

public slots:
	void SlotShowInspectionTime(double Value);
	void SlotShowCameraFramesPerSecond(double value);
	void SlotSetCameraStatus(const QString &Text,bool Simulation);
	void SlotStartToggelTimerShowLiveView();
	void SlotShowLiveImageTimeoutToggelTime();
	void SlotHoseFound(bool);

private:
	int                               m_CameraIndex;
	bool                              m_ResumeMeasuring;
	bool                              m_CheckNewReference;
	bool                              m_TerminateSaveResults;
	int                               m_CameraWidthInPixel;
	int                               m_CameraHeightInPixel;
	unsigned long                     m_FormatCounter;
	double                            m_FullInspectionTimeInms;
	MainAppPrintCheck                *m_MainAppPrintCheck;
	ImageMetaData                     m_LiveCameraImage;
	LiveImageView                    *m_FormatImageView;
	ErrorImageView                   *m_ErrorImageView;
	LiveImageView                    *m_ReferenceImageView;
	CameraImageView                  *m_CameraImageView;
	MeasureTaskDetectHose            *m_MeasureTaskDetectHose;
	MeasureTaskPrintCheck            *m_MeasureTaskPrintCheck;
	MeasureTaskFormatCheck           *m_MeasureTaskFormatCheck;
	MatchingShapeBased               *m_MatchingShapeBased;
	MatchingShapeBased               *m_MatchingFormatWindow;
	PrintLineDetector                *m_PrintLineDetector;
	ListResultsInspectionWindows      m_ListResultsInspectionWindows;//key is InspectionWindowID
	ListResultsSubFormat              m_ListResultsSubFormat;//key is ImageID
	QHash <int, ListResultsSubFormat> m_ListResultsFullHose;//key is HoseID
	TableWidgetSubFormatResults      *m_TableWidgetSubFormatResults;
	WidgetDetailResults              *m_WidgetDetailResults;
	QStringList                       m_HeaderListInspectionResults;
	QElapsedTimer                     m_TimerMeasureFullInspectionTime;
	QVector<QRgb>                     m_Colors;
	CameraSimulation                 *m_CameraSimulation;
	//QListWidget                      *m_InfoResults;
	QQueue<ImageMetaData>             m_QQueueSaveImageAndResults;
	QMutex                            m_MutexSaveImageAndResults;
	QWaitCondition                    m_WaitConditionSaveResults;
	QWaitCondition                    m_WaitConditionWaitForFinshed;
};

