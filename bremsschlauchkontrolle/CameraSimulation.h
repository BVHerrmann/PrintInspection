#pragma once
#include <QtCore>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"


class ImageData;
class CameraSimulation : public QThread
{
	Q_OBJECT
public:
	CameraSimulation(ImageData *pImageData);
	~CameraSimulation();
	virtual void   run();
	ImageData *GetImageData() { return m_ImageData; }
	void WaitForFinshed();
	void StartSimulation();
	void IncrementImageIndex()  { m_ImageIndexSubImageFromFile++; }
	void DecrementImageIndex()  { m_ImageIndexSubImageFromFile--; }
	void SetImageIndexSubImageFromFile(int set) { m_ImageIndexSubImageFromFile = set; }
	void SetVideoState(int set) { m_VideoState = set; }
	int  SetSimulationFrameInterval(int set) { m_SimulationFrameInterval = set; }
	bool SetNewImageFromFile();
	void SetStepOneImage() { m_StepOneImage = true; }
	int  GetNumberSubImageInFile() { return m_NumberSubImageInFile; }
	void ReadImageMetaData(QString &PathAndName);

signals:
	void SignalSetCameraStatus(const QString &Text,bool Simulation);
	void SignalSetSliderValues(int MaxValue,int Number,int CameraIndex);
	void SignalShowMessage(const QString &ErrorMsg, QtMsgType msgType);

private:
	ImageData      *m_ImageData;
	bool            m_TerminateInspection;
	bool            m_FirstImageInFile;
	bool            m_StepOneImage;
	bool            m_ImageMetaDataTopCameraFirst;
	QMutex          m_WaitThreadIsFinished;
	QWaitCondition  m_WaitConditionThreadIsFinished;
	cv::Mat         m_FullHoseImage;
	int             m_SimulationFrameInterval;
	int             m_ImageIndexSubImageFromFile;
	int             m_VideoState;
	int             m_NumberSubImageInFile;
    int             m_ImageMetaDataSubImageWidth;//entspricht der Kameraauflösung
	int             m_ImageMetaDataSubImageHeight;//entspricht der Kameraauflösung
	double          m_ImageMetaDataPixelSize;
};

