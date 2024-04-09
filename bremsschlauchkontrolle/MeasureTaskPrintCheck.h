#pragma once
#include <QtCore>
#include "ImageMetaData.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "halconcpp/HalconCpp.h"

class SharedMemoryVideoData;
class ImageData;
class MeasurementResult;
class MeasureTaskPrintCheck : public QThread
{
	Q_OBJECT
public:
	MeasureTaskPrintCheck(ImageData *pImageData);
	~MeasureTaskPrintCheck();
	ImageData *GetImageData() { return m_ImageData; }
	virtual void   run();
	void WaitForFinshed();
	int WaitForNewImageInSharedMemory();
	void StartPrintCheck(HalconCpp::HImage  &InputImage,int W,int H);// cv::Mat &m_FormatImage);
	HalconCpp::HImage       *GetHalconRGBResultImage() { return &(m_HalconRGBResultImage); }
	QImage CopyHalconImageIntoQtImage(HalconCpp::HImage  &HalconImage, int NumberChannelsQtImage);
	QVector<QRgb> GetColorTable() { return m_Colors; }
	bool CalculateFormatResults();
	void OverPaintMiddleLine();
	void OverPaintYOffset(int YOffset, int Xpos, int ModelLenght);
	void OverPaintModelNotFound(int xpos, int ypos, int ModelWidth, int ModelHeight);
	void SetEnableMeasuring(bool set) {m_EnableMeasuring = set;}
	void StartInspection();
	void OverPaintDefectRegion(InspectionWindow *pInspectionWindow);
	void DetachSharedMemoryInPrintchecktask();

signals:
	void SignalShowMessage(const QString &ErrorMsg, QtMsgType msgType);
	void SignalShowLiveImage(const ImageMetaData &Image);
	void SignalShowNewReferenceImage(const ImageMetaData &Image);
	void SignalShowHalconResultImage();
	void SignalShowErrorImage(const ImageMetaData &Image);
	

private:
	ImageData              *m_ImageData;
	ImageMetaData           m_FormatImageAndResults;
	ImageMetaData           m_SubFormatErrorImageAndResults;
	SharedMemoryVideoData  *m_SharedMemoryFormatImage;
	bool                    m_TerminateInspection;
	bool                    m_EnableMeasuring;
	QMutex                  m_WaitLiveImageViewIsDisable;
	QWaitCondition          m_WaitConditionLiveViewIsDisable;
	HANDLE                  m_EventTerminateThread;
	HalconCpp::HImage       m_HalconRGBResultImage;
	QVector<QRgb>           m_Colors;
	InspectionWindow        m_ResultsFormatImage;
	unsigned __int64        m_TimeStampFormatImageInMuSec;
	int                     m_FormatImageWidth;
	int                     m_FormatImageHeight;
	double                  m_HoseMiddlePosition;
};

