#pragma once
#include <QtCore>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "VideoHeader.h"
#include "MeasurementResult.h"
#include "ImageMetaData.h"
#include "CameraImageAndTimeStamps.h"

#include "bmessagebox.h"


class ImageData;
class SharedMemoryVideoData;
class HoseDetector;
class MeasureTaskDetectHose : public QThread
{
	Q_OBJECT
public:
	MeasureTaskDetectHose(ImageData *pImageData);
	~MeasureTaskDetectHose();
	ImageData *GetImageData() { return m_ImageData; }
	virtual void   run();
	void WaitForFinshed();
	void ClearSharedMemory();
	int PutNewReferenceImage(QString &ErrorMsg);
	HoseDetector  *GetHoseDetector() { return m_HoseDetector; }
	void NewIncommingImage(const cv::Mat &image,bool FromCamera=true);
	int DetectHose(CameraImageAndTimeStamps &cameraImageAndTimeStamps, QString &ErrorMsg);
	void LoadSettings();
	void GenerateReferenceData();
	void LoadAndShowReferenceImageFromDisk();
	void StartInspection();
	int CheckSharedMemoryFullHose(QString &ErrorMsg);
	int AppendNewImageIntoSharedMemory(CameraImageAndTimeStamps &cameraImageAndTimeStamps, QString &ErrorMsg);
	void AppendAndAverageNewResult(MeasurementResult &Results);
	void WriteFullHoseImageOnDisk();
	void DedectHoseAndAddImage(CameraImageAndTimeStamps &cameraImageAndTimeStamps);
	int GetNumberImagesInSharedMemory() { return m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames; }
	void SetNumberImagesInSharedMemory(int set) { m_SharedMemoryHeaderHoseImage.m_CurrentNumberFrames = set; }
	int GetSharedMemoryHeaderHoseImageStartPosNewFormat() { return m_SharedMemoryHeaderHoseImage.m_StartPosNewFormat; }
	int GetSharedMemoryHeaderHoseImageFormatEndXpos(){return m_SharedMemoryHeaderHoseImage.m_FormatEndXpos;}
	void SetSharedMemoryHeaderHoseImageStartPosNewFormat(int set, int FormatEndPos = 0);
	unsigned __int64 GetImageTimeStampByImageIndex(int index);
	MeasurementResult GetCurrentResultsHoseDetection() {return m_SharedMemoryHeaderHoseImage.m_ResultsHoseDetection;}
	void CalculateFramesPerSecond();
	void SetEnableWriteFullHose(bool set);
	bool GetEnableWriteFullHose() { return m_EnableWriteFullHose; }
	void SetDataToPLCHoseNotFound(unsigned __int64 m_TimeStamp);
	void GetReferenceSize(int &w, int &h);
	double GetCalculatetPixelSize() { return m_CalculatetPixelSize; }
	void EventFromPLCTubeEndIsReached();
	void IncrementHoseCounterAndClearSharedMemory();
	void SetStateHoseFound();
	void SetStateHoseNotFound();

signals:
	void SignalShowMessage(const QString &ErrorMsg, QtMsgType msgType);
	void SignalShowLiveImage(const ImageMetaData &Image);
	void SignalShowNewReferenceImage(const ImageMetaData &Image);
	void SignalNewReferenceDataGenerate(int CameraIndex,const QString &ErrorMsg);
	void SignalShowInspectionTime(double time);
	void SignalShowCameraFramesPerSecond(double time);
	void SignalWriteFullHoseImageIsFinished(int CameraIndex);
	void SignalShowCalculatetPixelSize(double PixelSize,double Diameter,int CameraIndex);
	void SignalShowMessageBox();
	void SignalHideMessageBox();
	void SignalHoseFound(bool);
	void SignalMeasuringIsStopped(int CameraIndex);

public slots:
	void SlotShowMessageBox();
	void SlotHideMessageBox();

private:
	ImageData                        *m_ImageData;
	VideoHeader                       m_SharedMemoryHeaderHoseImage;
	QMutex                            m_WaitLiveImageViewIsDisable;
	QWaitCondition                    m_WaitConditionLiveViewIsDisable;
	QMutex                            m_MutexNewImage;
	QWaitCondition                    m_WaitConditionNewImage;
	SharedMemoryVideoData            *m_SharedMemoryFullHose;
	HoseDetector                     *m_HoseDetector;
	ImageMetaData                     m_LiveCameraImageGUI;
	ImageMetaData                     m_ReferenceImageGUI;
	QList< MeasurementResult>         m_ListMeasurementResultsHoseDetection;
	MeasurementResult                 m_CurrentAverageResultsHoseDetection;
	HalconCpp::HImage                 m_ReferenceImage;
	bool                              m_TerminateInspection;
	bool                              m_EnableGenerateNewReferenceImage;
	bool                              m_StartInspectionIsClicked;
	bool                              m_ClearBuffer;
	bool                              m_FirstCallCalculateFramesPerSecond;
	bool                              m_EnableWriteFullHose;
	bool                              m_EventFromPLCTubeEndIsReached;
	bool                              m_EnableGetNewImage;
	int                               m_TimeoutValueWaitForNextImage;
	int                               m_ImageCounter;
	int                               m_ResultHoseDetection;
	int                               m_NumberImagesAverageHosePositionResults;
	int                               m_CameraFrameCounter;
	int                               m_SaveImageCounter;
	int                               m_CounterHoseNotFound;
	double                            m_CalculatetPixelSize;
	QQueue<CameraImageAndTimeStamps>  m_QQueueIncommingImages;
	QElapsedTimer                     m_TimerCameraIntervall;
	QString                           m_FileLocationSaveFullHose;
	BMessageBox                      *m_MessageBox;
};

