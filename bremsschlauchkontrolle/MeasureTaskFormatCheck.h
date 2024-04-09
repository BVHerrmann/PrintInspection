#pragma once
#include <QtCore>
#include "VideoHeader.h"
#include "qt_windows.h"
#include "ImageMetaData.h"
#include "halconcpp/HalconCpp.h"

class MeasurementResult;
class InspectionWindow;
class ImageData;
class SharedMemoryVideoData;
class MeasureTaskFormatCheck :	public QThread
{
	Q_OBJECT
public:
	MeasureTaskFormatCheck(ImageData *pImageData);
	~MeasureTaskFormatCheck();
	ImageData *GetImageData() { return m_ImageData; }
	virtual void   run();
	void WaitForFinshed();
	int WaitForNewImageInSharedMemory();
	void StartInspection();
	int CheckSharedMemorySizeFormatImage(int NewWidth, int NewHeight, QString &ErrorMsg);
	int AddFormatImage(HalconCpp::HImage  &FormatImageImage, InspectionWindow *pInspectionWindow, unsigned __int64 ImageTimeStampInMuSec, double eHoseMiddlePosition,QString &ErrorMsg);
	int CheckFormat(InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	bool CheckIsFormatInTolerance(InspectionWindow *pInspectionWindow);
	void  SetFormatNotFoundCounter(int set) { m_FormatNotFoundCounter = set; }
	bool  CalculateFormatLenght(InspectionWindow *pInspectionWindowdouble,double XRightPos, int StartPos);
	void SetLastXPosInHoseCoordinates(double set) { m_LastXPosInHoseCoordinates=set; }
	void AppendMatchSocoreAndAverage(InspectionWindow *pInspectionWindow);
	

signals:
	void SignalShowMessage(const QString &ErrorMsg, QtMsgType msgType);
	void SignalShowFormatNotFoundImage(const ImageMetaData &Image);
	

private:
	bool                      m_TerminateInspection;
	ImageData                *m_ImageData;
	SharedMemoryVideoData    *m_SharedMemoryFullHose;
	SharedMemoryVideoData    *m_SharedMemoryFormatImage;
	QMutex                    m_MutexThreadIsFinished;
	QWaitCondition            m_WaitConditionThreadIsFinished;
	HANDLE                    m_EventTerminateThread;
	VideoHeader               m_VideoHeaderFormatImage;
	double                    m_ROIRimInPixel;
	double                    m_LastXPosInHoseCoordinates;
	bool                      m_ReadDirectionLeftToRight;
	ImageMetaData             m_FormatErrorImage;
	int                       m_FormatNotFoundCounter;
	QList<double>             m_ListMatchScoresForamtDetection;
	//int  m_FormatCounter;
};

