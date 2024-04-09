#include "CameraSimulation.h"
#include "ImageData.h"
#include "MeasureTaskDetectHose.h"
#include "GlobalConst.h"
#include "MainAppPrintCheck.h"



CameraSimulation::CameraSimulation(ImageData *pImageData) : QThread()
, m_TerminateInspection(false)
, m_ImageData(NULL)
, m_ImageIndexSubImageFromFile(0)
, m_VideoState(PLAY_VIDEO)
, m_FirstImageInFile(true)
, m_StepOneImage(false)
, m_NumberSubImageInFile(0)
, m_ImageMetaDataPixelSize(0.0)
, m_ImageMetaDataSubImageWidth(0)
, m_ImageMetaDataSubImageHeight(0)
, m_ImageMetaDataTopCameraFirst(false)
, m_SimulationFrameInterval(70)
{
	m_ImageData = pImageData;
}


CameraSimulation::~CameraSimulation()
{
   WaitForFinshed();
}


void CameraSimulation::StartSimulation()
{
	m_TerminateInspection = false;
	start();
}


void CameraSimulation::WaitForFinshed()
{
	if (isRunning())
	{//thread läuft noch
		m_TerminateInspection = true;
		m_WaitThreadIsFinished.lock();
		m_WaitConditionThreadIsFinished.wait(&m_WaitThreadIsFinished, 15000);
		m_WaitThreadIsFinished.unlock();
	}
}

//Laden der Videodateien und das Bild in einzelene Blöcke zerteilen und dann an die Inspektion Übergeben
void CameraSimulation::run()
{
	QString MetaData,PathAndFileNameFullHoseImage,FileLocation;
	QDir Path;
	QStringList filters;
	QFileInfoList list;
	bool EndImage=false;
	
	if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
		FileLocation = GetImageData()->GetMainAppPrintCheck()->GetVideoFileLocationCameraTop();
	else
		FileLocation = GetImageData()->GetMainAppPrintCheck()->GetVideoFileLocationCameraBot();
	Path.setPath(FileLocation);
	filters << QString("*.%1").arg(VIDEO_FILE_EXTENSION);
	Path.setFilter(QDir::Files);
	Path.setNameFilters(filters);
	list = Path.entryInfoList();
	
	while (!m_TerminateInspection)
	{
		for (int i = 0; i < list.count(); i++)
		{
			emit SignalSetCameraStatus(tr("Camera Simulation (%1)!").arg(list.at(i).baseName()), true);
			m_FirstImageInFile = true;
			GetImageData()->EventFromPLCTubeEndIsReached();//Schlauchzähler um eins erhöhen Simulation Schlauchende
			PathAndFileNameFullHoseImage = list.at(i).filePath();
			ReadImageMetaData(PathAndFileNameFullHoseImage);
			
			m_FullHoseImage = cv::imread(PathAndFileNameFullHoseImage.toLatin1().data(), cv::IMREAD_GRAYSCALE);
			if (m_FullHoseImage.data)
			{
				EndImage=false;
				m_NumberSubImageInFile = m_FullHoseImage.rows / GetImageData()->GetCameraHeightInPixel();
				m_ImageIndexSubImageFromFile = 0;
				while (!EndImage)
				{//durchlauf einer Datei
					m_SimulationFrameInterval=GetImageData()->GetSimulationCameraIntervalInms();
					if (m_VideoState == PLAY_VIDEO)
					{
						EndImage = SetNewImageFromFile();
					}
					else
					{
						if (m_StepOneImage)//einzeschritt
						{
							EndImage = SetNewImageFromFile();
							m_StepOneImage = false;
						}
					}
					msleep(m_SimulationFrameInterval);
					if (m_TerminateInspection)
						break;
				}
			}
			if (m_TerminateInspection)
				break;
		}
		msleep(m_SimulationFrameInterval);
		if (m_TerminateInspection)
			break;
		if (list.count() == 0)
		{
			emit SignalShowMessage(tr("Can Not Run Simulation! No Video Files(*.%1) in dir:%2").arg(VIDEO_FILE_EXTENSION).arg(FileLocation), QtMsgType::QtFatalMsg);
		}
	}
    m_WaitConditionThreadIsFinished.wakeAll();
	emit SignalSetCameraStatus(tr("Camera Live Image"),false);
}

//Parameter die mit dem Bild verbunden sind
void CameraSimulation::ReadImageMetaData(QString &PathAndName)
{
	QImageReader ImageReader(PathAndName);
	bool ok;

	m_ImageMetaDataPixelSize = ImageReader.text(QString("PixelSize")).toDouble(&ok);
	if (ok)
	{
		if (GetImageData()->GetPixelSize() != m_ImageMetaDataPixelSize)
			GetImageData()->SetPixelSize(m_ImageMetaDataPixelSize);
	}
	m_ImageMetaDataSubImageWidth  = ImageReader.text(QString("SubImageWidth")).toInt(&ok);
	if (ok)
	{
		if (GetImageData()->GetCameraWidthInPixel() != m_ImageMetaDataSubImageWidth)
			GetImageData()->SetCameraWidthInPixel(m_ImageMetaDataSubImageWidth);
	}
	m_ImageMetaDataSubImageHeight = ImageReader.text(QString("SubImageHeight")).toInt(&ok);
	if (ok)
	{
		if (GetImageData()->GetCameraHeightInPixel() != m_ImageMetaDataSubImageHeight)
			GetImageData()->SetCameraHeightInPixel(m_ImageMetaDataSubImageHeight);
	}
	m_ImageMetaDataTopCameraFirst = static_cast<bool>(ImageReader.text(QString("TopCameraFirst")).toInt(&ok));
	if (ok)
	{
		if (GetImageData()->GetMainAppPrintCheck()->GetTopCameraIsFirst() != m_ImageMetaDataTopCameraFirst)
			GetImageData()->GetMainAppPrintCheck()->SetTopCameraIsFirst(m_ImageMetaDataTopCameraFirst);
	}
}

//Bild an die MeasureTaskDetectHose
bool CameraSimulation::SetNewImageFromFile()
{
	bool EndImage = false;
	cv::Rect ROIRect;
	cv::Mat ROIImage;

	ROIRect.width = GetImageData()->GetCameraWidthInPixel();
	ROIRect.height = GetImageData()->GetCameraHeightInPixel();
	ROIRect.x = 0;
	
	if (m_ImageIndexSubImageFromFile >= m_NumberSubImageInFile)
	{
		EndImage = true;
	}
	else
	{
		ROIRect.y = m_ImageIndexSubImageFromFile * (ROIRect.height - 1);
	    m_ImageIndexSubImageFromFile++;
		GetImageData()->CopyROIImage(ROIRect, m_FullHoseImage, ROIImage);
		emit SignalSetSliderValues(m_NumberSubImageInFile, m_ImageIndexSubImageFromFile, GetImageData()->GetCameraIndex());
		if (!ROIImage.empty())
			GetImageData()->GetMeasureTaskDetectHose()->NewIncommingImage(ROIImage, false);
	}
	return EndImage;
}
