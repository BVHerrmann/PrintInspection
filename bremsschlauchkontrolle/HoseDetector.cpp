#include "HoseDetector.h"
#include "MeasurementResult.h"
#include "GlobalConst.h"
#include "ImageData.h"


HoseDetector::HoseDetector(ImageData *pImageData) : QObject()
,m_ModelID(0)
{
	m_ImageData                                          = pImageData;
	m_ModelParameter.m_ModelHeightInPixel                = GetImageData()->GetCameraHeightInPixel(); 
	m_ModelParameter.m_ModelWidthInPixel                 = GetImageData()->GetHoseDiameterInPixel(); 
	m_ModelParameter.m_MaxModelScaleFactor               = 1.1;
	m_ModelParameter.m_MinModelScaleFactor               = 0.9;
	m_ModelParameter.m_ModelAcceptanceThresholdInPercent = 0.5;
	m_ModelParameter.m_SearchAngleRangeNegInDegree       = -5.0;
	m_ModelParameter.m_SearchAngleRangePosInDegree       = 5.0;
	m_ModelParameter.m_MinScore                          = 0.3;
}


HoseDetector::~HoseDetector()
{
	if (m_ModelID != 0)
	{
		HalconCpp::ClearShapeModel(m_ModelID);
		m_ModelID = 0;
	}
}


void HoseDetector::LoadSettings()
{
	QString LocationMeasuringSettings = GetImageData()->GetReferenceLocation() + QString("/") + MEASURING_PARAMETER_FILE_NAME;
	QSettings settings(LocationMeasuringSettings, QSettings::IniFormat);
	QString CameraName;

	if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_BOT_DIR_NAME;
	
	m_ModelParameter.m_MaxModelScaleFactor                 = settings.value(QString("HoseDetector%1/MaxModelScaleFactor").arg(CameraName),               m_ModelParameter.m_MaxModelScaleFactor).toDouble();
	m_ModelParameter.m_MinModelScaleFactor                 = settings.value(QString("HoseDetector%1/MinModelScaleFactor").arg(CameraName),               m_ModelParameter.m_MinModelScaleFactor).toDouble();
	m_ModelParameter.m_ModelAcceptanceThresholdInPercent   = settings.value(QString("HoseDetector%1/ModelAcceptanceThresholdInPercent").arg(CameraName), m_ModelParameter.m_ModelAcceptanceThresholdInPercent).toDouble();
	m_ModelParameter.m_SearchAngleRangeNegInDegree         = settings.value(QString("HoseDetector%1/SearchAngleRangeNegInDegree").arg(CameraName),       m_ModelParameter.m_SearchAngleRangeNegInDegree).toDouble();
	m_ModelParameter.m_SearchAngleRangePosInDegree         = settings.value(QString("HoseDetector%1/SearchAngleRangePosInDegree").arg(CameraName),       m_ModelParameter.m_SearchAngleRangePosInDegree).toDouble();
	m_ModelParameter.m_MinScore                            = settings.value(QString("HoseDetector%1/MinScore").arg(CameraName),                          m_ModelParameter.m_MinScore).toDouble();
	SaveSettings();
}



void HoseDetector::SaveSettings()
{
	QString LocationMeasuringSettings = GetImageData()->GetReferenceLocation() + QString("/") + MEASURING_PARAMETER_FILE_NAME;
	QSettings settings(LocationMeasuringSettings, QSettings::IniFormat);
	QString CameraName;

	if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_BOT_DIR_NAME;

	settings.setValue(QString("HoseDetector%1/MaxModelScaleFactor").arg(CameraName),               m_ModelParameter.m_MaxModelScaleFactor);
	settings.setValue(QString("HoseDetector%1/MinModelScaleFactor").arg(CameraName),               m_ModelParameter.m_MinModelScaleFactor);
	settings.setValue(QString("HoseDetector%1/ModelAcceptanceThresholdInPercent").arg(CameraName), m_ModelParameter.m_ModelAcceptanceThresholdInPercent);
	settings.setValue(QString("HoseDetector%1/SearchAngleRangeNegInDegree").arg(CameraName),       m_ModelParameter.m_SearchAngleRangeNegInDegree);
	settings.setValue(QString("HoseDetector%1/SearchAngleRangePosInDegree").arg(CameraName),       m_ModelParameter.m_SearchAngleRangePosInDegree);
	settings.setValue(QString("HoseDetector%1/MinScore").arg(CameraName),                          m_ModelParameter.m_MinScore);
}


//Schlauchsuche starten
int HoseDetector::StartDetection(HalconCpp::HImage &InputImage, QString &ErrorMsg)
{
	int rv= ERROR_CODE_NO_ERROR;
	try
	{
		HalconCpp::HTuple  ResultRow, ResultColumn, ResultAngle, ResultScale, ResultScore;
		InspectionWindow *pInspectionWindowHoseDetection = GetImageData()->GetInspectionWindowHoseDetection();
		
		if (pInspectionWindowHoseDetection)
		{
			if (pInspectionWindowHoseDetection->m_ModelWidthReference != m_ModelParameter.m_ModelWidthInPixel || pInspectionWindowHoseDetection->m_ModelHeightReference != m_ModelParameter.m_ModelHeightInPixel || m_ModelID == 0)
			{
				m_ModelParameter.m_ModelWidthInPixel = pInspectionWindowHoseDetection->m_ModelWidthReference;
				m_ModelParameter.m_ModelHeightInPixel = pInspectionWindowHoseDetection->m_ModelHeightReference;// HoseDiameterInPixel;
				rv = GenerateSyntehticModelHose(ErrorMsg);//Modell für den Schlauch muss nur einmal erstellt werden
			}
			if (rv == ERROR_CODE_NO_ERROR)
			{
				double XTopLeft, YTopLeft, XBottomRight, YBottomRight;
				//ROI bestimmen
				YTopLeft     = pInspectionWindowHoseDetection->m_ReferenceRect.topLeft().y();
				YBottomRight = pInspectionWindowHoseDetection->m_ReferenceRect.bottomRight().y();
				XTopLeft     = pInspectionWindowHoseDetection->m_ReferenceRect.topLeft().x();
				XBottomRight = pInspectionWindowHoseDetection->m_ReferenceRect.bottomRight().x();
				//ROI Kopieren		   			 			
				HalconCpp::HImage ROIImage = InputImage.CropRectangle1((Hlong)(YTopLeft), (Hlong)(XTopLeft), (Hlong)(YBottomRight - 1), (Hlong)(XBottomRight - 1));
				//Schlauch suchen, position und größe
				FindScaledShapeModel(ROIImage, m_ModelID, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinScore, 1, 1, "none", (HalconCpp::HTuple(5).Append(3)), 0.9, &ResultRow, &ResultColumn, &ResultAngle, &ResultScale, &ResultScore);
				if (ResultScore.Length() >= 1)
				{//wurde etwas gefunden
					if (ResultScore[0].D() > m_ModelParameter.m_ModelAcceptanceThresholdInPercent)//passt die Schwelle
						pInspectionWindowHoseDetection->m_Results.m_ModelFound = true;
					else
						pInspectionWindowHoseDetection->m_Results.m_ModelFound = false;
					pInspectionWindowHoseDetection->m_Results.m_ModelScore             = ResultScore[0].D() * 100.0;//in percent
					pInspectionWindowHoseDetection->m_Results.m_ResultYPos             = ResultRow[0].D() + YTopLeft;
					pInspectionWindowHoseDetection->m_Results.m_ResultXPos             = ResultColumn[0].D() + XTopLeft;
					pInspectionWindowHoseDetection->m_Results.m_ResultScaleFactorInX   = ResultScale[0].D();
					pInspectionWindowHoseDetection->m_Results.m_ObjectSizeInY          = pInspectionWindowHoseDetection->m_ModelHeightReference * pInspectionWindowHoseDetection->m_Results.m_ResultScaleFactorInX;//Y spielt hier keine Rolle da nur die Schlauchdicke(hier x Richtung) von bedeutung
					pInspectionWindowHoseDetection->m_Results.m_ObjectSizeInX          = pInspectionWindowHoseDetection->m_ModelWidthReference  * pInspectionWindowHoseDetection->m_Results.m_ResultScaleFactorInX;
				}
				else
					pInspectionWindowHoseDetection->m_Results.m_ModelFound = false;//nichts gefunden
			}
		}
	}
	catch (HalconCpp::HException &exception)
	{//schwerwiegender Fehler
		ErrorMsg = tr("Error In DetectHose. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
    return rv;
}

//Modell des Schlauche erstellen
int HoseDetector::GenerateSyntehticModelHose(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	try
	{
		HalconCpp::HImage SyntheticTemplateImage;
		HalconCpp::HObject   Rectangle, RectContour;
		int TemplateRim = 32;
		int Offset = 10;
		HalconCpp::HTuple SyntheticTemplateWidth = m_ModelParameter.m_ModelWidthInPixel + TemplateRim * 2;//ist der Durchmesser
		HalconCpp::HTuple SyntheticTemplateHight = m_ModelParameter.m_ModelHeightInPixel;//is CameraHeight
		
		
		if (SyntheticTemplateWidth > GetImageData()->GetCameraWidthInPixel())
		{
			TemplateRim = SyntheticTemplateWidth - GetImageData()->GetCameraWidthInPixel();
			SyntheticTemplateWidth = m_ModelParameter.m_ModelWidthInPixel + TemplateRim * 2;
		}
		if (m_ModelID != 0)
		{
			HalconCpp::ClearShapeModel(m_ModelID);
			m_ModelID = 0;
		}
		GenImageConst(&SyntheticTemplateImage, "byte", SyntheticTemplateWidth, SyntheticTemplateHight);
		GenRectangle1(&Rectangle, 0, 0, SyntheticTemplateHight - 1, SyntheticTemplateWidth - 1);
		PaintRegion(Rectangle, SyntheticTemplateImage, &SyntheticTemplateImage, 255.0, "fill");
		GenRectangle2ContourXld(&RectContour, SyntheticTemplateHight / 2.0, SyntheticTemplateWidth/ 2.0, 0.0, m_ModelParameter.m_ModelWidthInPixel / 2.0, (SyntheticTemplateHight + Offset) / 2.0);//SyntheticTemplateWidth + 10 damit die recht und linke Kante verschwindet
		PaintXld(RectContour, SyntheticTemplateImage, &SyntheticTemplateImage, 0);
		CreateScaledShapeModel(SyntheticTemplateImage, "auto", HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", "auto", "use_polarity", "auto", "auto", &m_ModelID);
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In Generate Model Hose.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}
















//old stuff
//not in use
int HoseDetector::GenerateSyntehticModelHose2(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	try
	{
		int TemplateRim = 32;

		HalconCpp::HTuple SyntheticTemplateWidth = m_ModelParameter.m_ModelWidthInPixel + TemplateRim * 2;//ist der Durchmesser
		
		HalconCpp::HTuple CameraWidth  = GetImageData()->GetCameraWidthInPixel();
		HalconCpp::HTuple CameraHeight = GetImageData()->GetCameraHeightInPixel();

		GenMeasureRectangle2(CameraHeight / 2.0, CameraWidth / 2.0, 0.0, CameraHeight / 2.0, SyntheticTemplateWidth / 2.0, CameraWidth, CameraHeight, "nearest_neighbor", &m_ModelID);
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In Generate Model Hose.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}

//not in use
int HoseDetector::StartDetection2(HalconCpp::HImage &InputImage, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	try
	{
		InspectionWindow *pInspectionWindowHoseDetection = GetImageData()->GetInspectionWindowHoseDetection();
		if (pInspectionWindowHoseDetection->m_ModelWidthReference != m_ModelParameter.m_ModelWidthInPixel || pInspectionWindowHoseDetection->m_ModelHeightReference != m_ModelParameter.m_ModelHeightInPixel || m_ModelID == 0)
		{
			m_ModelParameter.m_ModelWidthInPixel = pInspectionWindowHoseDetection->m_ModelWidthReference;
			m_ModelParameter.m_ModelHeightInPixel = pInspectionWindowHoseDetection->m_ModelHeightReference;// HoseDiameterInPixel;
			rv = GenerateSyntehticModelHose2(ErrorMsg);
		}
		if (rv == ERROR_CODE_NO_ERROR)
		{
			HalconCpp::HTuple hv_Sigma = 1.1;
			HalconCpp::HTuple hv_Threshold = 20;
			HalconCpp::HTuple hv_Transition = "negative";
			HalconCpp::HTuple hv_Select = "all";

			HalconCpp::HTuple   hv_RowEdgeFirst, hv_ColumnEdgeFirst;
			HalconCpp::HTuple  hv_AmplitudeFirst, hv_RowEdgeSecond, hv_ColumnEdgeSecond;
			HalconCpp::HTuple  hv_AmplitudeSecond, hv_IntraDistance, hv_InterDistance;


			MeasurePairs(InputImage, m_ModelID, hv_Sigma, hv_Threshold, hv_Transition, hv_Select, &hv_RowEdgeFirst, &hv_ColumnEdgeFirst, &hv_AmplitudeFirst, &hv_RowEdgeSecond, &hv_ColumnEdgeSecond, &hv_AmplitudeSecond, &hv_IntraDistance, &hv_InterDistance);

			if (hv_RowEdgeSecond.Length() >= 1)
			{
				double Amp = (hv_AmplitudeFirst[0].D() + hv_AmplitudeSecond[0].D()) / 2.0;
				pInspectionWindowHoseDetection->m_Results.m_ModelScore = Amp * (100.0 / 255.0);
				pInspectionWindowHoseDetection->m_Results.m_ResultYPos = (hv_RowEdgeSecond[0].D() + hv_RowEdgeFirst[0].D()) / 2.0;
				pInspectionWindowHoseDetection->m_Results.m_ResultXPos = (hv_ColumnEdgeSecond[0].D() + hv_ColumnEdgeFirst[0].D()) / 2.0;
				pInspectionWindowHoseDetection->m_Results.m_ObjectSizeInY = m_ModelParameter.m_ModelHeightInPixel;
				pInspectionWindowHoseDetection->m_Results.m_ObjectSizeInX = hv_IntraDistance[0].D();
				pInspectionWindowHoseDetection->m_Results.m_ModelFound = true;
			}
			else
				pInspectionWindowHoseDetection->m_Results.m_ModelFound = false;
		}
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In DetectHose. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}