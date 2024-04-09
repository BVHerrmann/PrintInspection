#include "PrintLineDetector.h"
#include "ImageData.h"
#include "GlobalConst.h"


PrintLineDetector::PrintLineDetector(ImageData *pImageData, const QString &MeasureToolName) : QObject()
, m_ImageData(NULL)
, m_ImageCounter(0)
, m_ModelID(0)
{
	m_ImageData                           = pImageData;
	m_MeasureToolName                     = MeasureToolName;
	m_ModelParameter.m_ModelWidthInPixel  = 0;
	m_ModelParameter.m_ModelHeightInPixel = 0;

	m_ModelParameter.m_GrayThreshold        = 100;//hier nicht genutzt
	m_ModelParameter.m_MaxModelScaleFactor  = 1.05;//hier x
	m_ModelParameter.m_MinModelScaleFactor  = 0.95;//hier x
	m_ModelParameter.m_MaxModelScaleFactorY = 1.0;
	m_ModelParameter.m_MinModelScaleFactorY = 1.0;
	m_ModelParameter.m_ModelAcceptanceThresholdInPercent = 0.5;
	m_ModelParameter.m_SearchAngleRangeNegInDegree = -3.0;
	m_ModelParameter.m_SearchAngleRangePosInDegree = 3.0;
	m_ModelParameter.m_MinScore = 0.5;
	m_ModelParameter.m_ThresholToleranceFactor = 60.0;//
}


PrintLineDetector::~PrintLineDetector()
{
	ClearModel();
}


void PrintLineDetector::ClearModel()
{
	if (m_ModelID != 0)
	{
		HalconCpp::ClearShapeModel(m_ModelID);
		m_ModelID = 0;
	}
}


int  PrintLineDetector::StartDetection(HalconCpp::HImage &InputImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;

	try
	{
		HalconCpp::HTuple  ResultRow, ResultColumn, ResultAngle, ResultScaleR, ResultScaleC,ResultScore;
		if (pInspectionWindow->m_ModelWidthReference != m_ModelParameter.m_ModelWidthInPixel || pInspectionWindow->m_ModelHeightReference != m_ModelParameter.m_ModelHeightInPixel || m_ModelID == 0)
		{
			m_ModelParameter.m_ModelWidthInPixel  = pInspectionWindow->m_ModelWidthReference;
			m_ModelParameter.m_ModelHeightInPixel = pInspectionWindow->m_ModelHeightReference;
			rv = GenerateSyntehticModelRectangle(ErrorMsg);
		}
		if (rv == ERROR_CODE_NO_ERROR)
		{
			double XTopLeft, YTopLeft, XBottomRight, YBottomRight;
			//ROI bestimmen
			XTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().x();
			YTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().y();
			XBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().x() - 1;
			YBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().y() - 1;
			//ROI Kopieren		   			 			
			HalconCpp::HImage ROIImage = InputImage.CropRectangle1((Hlong)(YTopLeft), (Hlong)(XTopLeft), (Hlong)(YBottomRight), (Hlong)(XBottomRight));
			GrayClosingRect(ROIImage, &ROIImage, 3, 3);//Schliessen von Lüken
			FindAnisoShapeModel(ROIImage, m_ModelID, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactorY, m_ModelParameter.m_MaxModelScaleFactorY, m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinScore, 1, 1, "interpolation", (HalconCpp::HTuple(5).Append(3)), 0.1, &ResultRow, &ResultColumn, &ResultAngle, &ResultScaleR, &ResultScaleC, &ResultScore);
			if (ResultScore.Length() >= 1)
			{
				if (ResultScore[0].D() > m_ModelParameter.m_ModelAcceptanceThresholdInPercent)
					pInspectionWindow->m_Results.m_ModelFound = true;
				else
					pInspectionWindow->m_Results.m_ModelFound = false;
				pInspectionWindow->m_Results.m_ModelScore = ResultScore[0].D() * 100.0;//in percent
				pInspectionWindow->m_Results.m_ResultYPos = ResultRow[0].D();// +YTopLeft;
				pInspectionWindow->m_Results.m_ResultXPos = ResultColumn[0].D();// +XTopLeft;
				pInspectionWindow->m_Results.m_ResultScaleFactorInX = ResultScaleC[0].D();
				pInspectionWindow->m_Results.m_ResultScaleFactorInY = ResultScaleR[0].D();
				pInspectionWindow->m_Results.m_ObjectSizeInY = pInspectionWindow->m_ModelHeightReference * pInspectionWindow->m_Results.m_ResultScaleFactorInY;//Y spielt hier keine Rolle da nur die Schlauchdicke(hier x Richtung) von bedeutung
				pInspectionWindow->m_Results.m_ObjectSizeInX = pInspectionWindow->m_ModelWidthReference  * pInspectionWindow->m_Results.m_ResultScaleFactorInX;
			}
			else
				pInspectionWindow->m_Results.m_ModelFound = false;
			pInspectionWindow->m_Results.m_ResultsValid = true;
			if (pInspectionWindow->m_Results.m_ModelFound)
			{//Block gefunden
				rv = CheckLineMissing(ROIImage, pInspectionWindow, ErrorMsg);
				pInspectionWindow->m_Results.m_ResultYPos = pInspectionWindow->m_Results.m_ResultYPos + YTopLeft;
				pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos + XTopLeft;
			}

		}
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In Generate Model Rectangle.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int PrintLineDetector::GenerateSyntehticModelRectangle(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	try
	{
		HalconCpp::HImage SyntheticTemplateImage;
		HalconCpp::HObject   Rectangle, RectContour;
		HalconCpp::HRegion ModelRegion;
		int TemplateRim = 8;
		HalconCpp::HTuple SyntheticTemplateWidth = m_ModelParameter.m_ModelWidthInPixel  + TemplateRim * 2;
		HalconCpp::HTuple SyntheticTemplateHight = m_ModelParameter.m_ModelHeightInPixel + TemplateRim * 2;
		
		
		ClearModel();
		GenImageConst(&SyntheticTemplateImage, "byte", SyntheticTemplateWidth, SyntheticTemplateHight);
		GenRectangle1(&Rectangle, 0, 0, SyntheticTemplateHight - 1, SyntheticTemplateWidth - 1);
		PaintRegion(Rectangle, SyntheticTemplateImage, &SyntheticTemplateImage, 0.0, "fill");
		GenRectangle2ContourXld(&RectContour, SyntheticTemplateHight / 2.0, SyntheticTemplateWidth / 2.0, 0.0, m_ModelParameter.m_ModelWidthInPixel / 2.0, m_ModelParameter.m_ModelHeightInPixel / 2.0);
		GenRegionContourXld(RectContour, &ModelRegion, "filled");
		PaintRegion(ModelRegion, SyntheticTemplateImage, &SyntheticTemplateImage, 255.0, "fill");
		CreateAnisoShapeModel(SyntheticTemplateImage, "auto", HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), "auto", m_ModelParameter.m_MinModelScaleFactorY, m_ModelParameter.m_MaxModelScaleFactorY, "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", "auto", "use_polarity", "auto", "auto", &(m_ModelID));
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In Generate Model Hose.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int PrintLineDetector::CheckLineMissing(HalconCpp::HImage &ROIImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	double XTopLeft     = pInspectionWindow->m_Results.m_ResultXPos - pInspectionWindow->m_ModelWidthReference / 2.0;
	double YTopLeft     = pInspectionWindow->m_Results.m_ResultYPos - pInspectionWindow->m_ModelHeightReference / 2.0;
	double XBottomRight = pInspectionWindow->m_Results.m_ResultXPos + pInspectionWindow->m_ModelWidthReference / 2.0;
	double YBottomRight = pInspectionWindow->m_Results.m_ResultYPos + pInspectionWindow->m_ModelHeightReference / 2.0;
	Hlong W, H;
	

	try
	{
		HalconCpp::HRegion BinRegion;
		HalconCpp::HImage LineImage, BlockImage;
		double DefectScore,LineHeight, YTop, YBot, MeanInPercent;
		double DMean,MaxDefectScore = 0.0;
		double LineThreshold = m_ModelParameter.m_ThresholToleranceFactor;
		HalconCpp::HTuple AbsoluteHisto, RelativeHisto,MeanValue;


		ROIImage.GetImageSize(&W, &H);
		if (YTopLeft < 0)YTopLeft = 0.0;
		if (XTopLeft < 0)XTopLeft = 0.0;
		if (YBottomRight > H)YBottomRight = H;
		if (XBottomRight > W)XBottomRight = W;
		BlockImage = ROIImage.CropRectangle1(YTopLeft, XTopLeft, YBottomRight - 1, XBottomRight - 1);
		BlockImage.GetImageSize(&W, &H);
		GrayClosingRect(BlockImage, &BlockImage, 3, 3);//Schliessen von Lücken
		LineHeight = double(H) / pInspectionWindow->m_NumberHorizontalLines;
		YTop = 0.0;
		YBot = LineHeight - 1;
		for (int i = 0; i < pInspectionWindow->m_NumberHorizontalLines; i++)
		{   //einzelne Zeilen auslesen
			LineImage = BlockImage.CropRectangle1(YTop, 0, YBot, W - 1);
			LineImage.GetImageSize(&W, &H);
			if (W > 0 && H > 0)
			{
				//Threshold(LineImage, &BinRegion, 0, ThresholdValue);//vorgabe threshold
				//RegionToBin(BinRegion, &LineImage, 0, 255, W, H);
				//QString Path = QString("d://Temp3//BinImage%1%2.bmp").arg(m_ImageCounter).arg(i);
				//LineImage.WriteImage("bmp", 0, Path.toLatin1().data());
				GrayFeatures(LineImage, LineImage, "mean", &MeanValue);
				DMean         = MeanValue[0].D();
				MeanInPercent = (100.0 / 255.0) * DMean;//
				DefectScore   = 100.0 - MeanInPercent;
				if (DefectScore > MaxDefectScore)
				{
					MaxDefectScore = DefectScore;
					pInspectionWindow->m_Results.m_DefectScore   = MaxDefectScore;
					pInspectionWindow->m_Results.m_MeanGrayValue = DMean;
				}
				if (DefectScore > LineThreshold)
				{// diese Zeile unterschreitet den Schwellwert
					pInspectionWindow->m_Results.m_LineCheckOk = false;
					pInspectionWindow->m_Results.m_DefectScore = DefectScore;
					break;
				}
			}
			YTop = YTop + LineHeight - 1;
			YBot = YBot + LineHeight - 1;
		}
	    m_ImageCounter++;
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In CheckLineMissing Cut BlockImage.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		return ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int  PrintLineDetector::GenerateModelReferenceData(HalconCpp::HImage &RefImageImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	double StripeWidthInPix = 2.5 / 0.06;

	InspectionWindow *pInspectionWindowFormat = GetImageData()->GetInspectionWindowByID(0);
	if (pInspectionWindowFormat)
	{
		HalconCpp::HImage ROIImage, FormatImage;
		double XTopLeft = pInspectionWindowFormat->m_ReferenceRect.topLeft().x();
		double YTopLeft = pInspectionWindowFormat->m_ReferenceRect.topLeft().y();
		double XBottomRight = pInspectionWindowFormat->m_ReferenceRect.bottomRight().x();
		double YBottomRight = pInspectionWindowFormat->m_ReferenceRect.bottomRight().y();
		QString ReferenceLocation = GetImageData()->GetReferenceLocation() + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID);
		QString Location, NameRefImage = "/" + REFERENCE_MODEL_IMAGE_FILE_NAME;


		QDir().mkpath(ReferenceLocation);
		try
		{
			FormatImage  = RefImageImage.CropRectangle1(YTopLeft, XTopLeft, YBottomRight - 1, XBottomRight - 1);
			XTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().x();
			YTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().y();
			XBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().x();
			YBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().y();
			ROIImage     = FormatImage.CropRectangle1(YTopLeft, XTopLeft, YBottomRight - 1, XBottomRight - 1);
		}
		catch (HalconCpp::HException &exception)
		{
			ErrorMsg = tr("Can Not Generate Model Ref Data In PrintLineDetector.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			return ERROR_CODE_ANY_ERROR;
		}
		Location = ReferenceLocation + NameRefImage;
		ROIImage.WriteImage("bmp", 0, Location.toLatin1().data());
		pInspectionWindow->m_HaveReferenceData    = true;
	}
	return rv;
}


void PrintLineDetector::LoadSettings()
{
	if (GetImageData())
	{
		QString LocationMeasuringSettings = GetImageData()->GetReferenceLocation() + QString("/") + MEASURING_PARAMETER_FILE_NAME;
		QSettings settings(LocationMeasuringSettings, QSettings::IniFormat);
		QString CameraName;

		if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
			CameraName = CAMERA_TOP_DIR_NAME;
		else
			CameraName = CAMERA_BOT_DIR_NAME;
		m_ModelParameter.m_MinContrast                       = settings.value(QString("PrintCheck%1/%2/MinContrast").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinContrast).toDouble();
		m_ModelParameter.m_GrayThreshold                     = settings.value(QString("PrintCheck%1/%2/GrayThreshold").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_GrayThreshold).toDouble();
		m_ModelParameter.m_MaxModelScaleFactor               = settings.value(QString("PrintCheck%1/%2/MaxModelScaleFactorX").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MaxModelScaleFactor).toDouble();
		m_ModelParameter.m_MinModelScaleFactor               = settings.value(QString("PrintCheck%1/%2/MinModelScaleFactorX").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinModelScaleFactor).toDouble();
		m_ModelParameter.m_MaxModelScaleFactorY              = settings.value(QString("PrintCheck%1/%2/MaxModelScaleFactorY").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MaxModelScaleFactorY).toDouble();
		m_ModelParameter.m_MinModelScaleFactorY              = settings.value(QString("PrintCheck%1/%2/MinModelScaleFactorY").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinModelScaleFactorY).toDouble();
		m_ModelParameter.m_ModelAcceptanceThresholdInPercent = settings.value(QString("PrintCheck%1/%2/ModelAcceptanceThresholdInPercent").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_ModelAcceptanceThresholdInPercent).toDouble();
		m_ModelParameter.m_SearchAngleRangeNegInDegree       = settings.value(QString("PrintCheck%1/%2/SearchAngleRangeNegInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangeNegInDegree).toDouble();
		m_ModelParameter.m_SearchAngleRangePosInDegree       = settings.value(QString("PrintCheck%1/%2/SearchAngleRangePosInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangePosInDegree).toDouble();
		m_ModelParameter.m_MinScore                          = settings.value(QString("PrintCheck%1/%2/MinScore").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinScore).toDouble();
		m_ModelParameter.m_ThresholToleranceFactor           = settings.value(QString("PrintCheck%1/%2/ThresholToleranceFactor").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_ThresholToleranceFactor).toDouble();
		SaveSettings();
	}
}


void PrintLineDetector::SaveSettings()
{
	if (GetImageData())
	{
		QString LocationMeasuringSettings = GetImageData()->GetReferenceLocation() + QString("/") + MEASURING_PARAMETER_FILE_NAME;
		QSettings settings(LocationMeasuringSettings, QSettings::IniFormat);
		QString CameraName;

		if (GetImageData()->GetCameraIndex() == CAMERA_TOP_INDEX)
			CameraName = CAMERA_TOP_DIR_NAME;
		else
			CameraName = CAMERA_BOT_DIR_NAME;
		settings.setValue(QString("PrintCheck%1/%2/MinContrast").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinContrast);
		settings.setValue(QString("PrintCheck%1/%2/GrayThreshold").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_GrayThreshold);
		settings.setValue(QString("PrintCheck%1/%2/MaxModelScaleFactorX").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MaxModelScaleFactor);
		settings.setValue(QString("PrintCheck%1/%2/MinModelScaleFactorX").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinModelScaleFactor);
		settings.setValue(QString("PrintCheck%1/%2/MaxModelScaleFactorY").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MaxModelScaleFactorY);
		settings.setValue(QString("PrintCheck%1/%2/MinModelScaleFactorY").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinModelScaleFactorY);
		settings.setValue(QString("PrintCheck%1/%2/ModelAcceptanceThresholdInPercent").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_ModelAcceptanceThresholdInPercent);
		settings.setValue(QString("PrintCheck%1/%2/SearchAngleRangeNegInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangeNegInDegree);
		settings.setValue(QString("PrintCheck%1/%2/SearchAngleRangePosInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangePosInDegree);
		settings.setValue(QString("PrintCheck%1/%2/MinScore").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinScore);
		settings.setValue(QString("PrintCheck%1/%2/ThresholToleranceFactor").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_ThresholToleranceFactor);
	}
}































//old stuff

/*int  PrintLineDetector::_StartDetection(HalconCpp::HImage &FormatImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	int DiffHeight;
	double ThresHoldInPercent,DiffInPercent;

	try
	{
		//rv=ReadReferenceBlockSize(pInspectionWindow, ErrorMsg);//Einlesen der Referenzdaten von Datei, hier nur die Blockhöhe. Datei wird erzeugt beim Anlegen einer Referenz 
		if (pInspectionWindow->m_ModelHeightReference>0 && pInspectionWindow->m_HaveReferenceData)
		{
			rv = GenerateMesureHandle(pInspectionWindow, ErrorMsg);//Definition des Kantendedektor
			if (rv == ERROR_CODE_NO_ERROR)
			{//ROI ausschneiden aus dem Formatbild in dem sich der Block befindet
				double XTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().x();
				double YTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().y();
				double XBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().x();
				double YBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().y();
				HalconCpp::HImage ROIImage = FormatImage.CropRectangle1(YTopLeft, XTopLeft, YBottomRight - 1, XBottomRight - 1);
				rv = CalculateTopAndBotLine(ROIImage, pInspectionWindow, ErrorMsg);//Suchen nach der oberen und unteren Kante und Bestimmung der Blockhöhe
				if (rv == ERROR_CODE_NO_ERROR)
				{
					if (pInspectionWindow->m_Results.m_ResultsValid && pInspectionWindow->m_Results.m_ModelFound)
					{//berechnete Blockhöhe mit der Referenz vergleichen
						DiffHeight = abs(pInspectionWindow->m_ModelHeightReference - pInspectionWindow->m_Results.m_ObjectSizeInY);
						DiffInPercent = (100.0 / pInspectionWindow->m_ModelHeightReference) * DiffHeight;
						ThresHoldInPercent = 100.0/pInspectionWindow->m_NumberHorizontalLines;
						if (DiffInPercent > (ThresHoldInPercent*1.5))//hier Faktor 1.5 um Fehlalarme zu vermeiden
							pInspectionWindow->m_Results.m_LineCheckOk = false;//Höhe stimmt nicht, hier entweder obere oder untere Reihe nicht bedruckt oder beid Reihen nicht bedruckt
						else
							rv=CheckLineMissing(ROIImage, pInspectionWindow, ErrorMsg);//Oben und Unten Ok, Prüfung ob in der Mitte Reihen nicht bedruckt
						pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos + XTopLeft;
						pInspectionWindow->m_Results.m_ResultYPos = pInspectionWindow->m_Results.m_ResultYPos + YTopLeft;
					}
					else
						pInspectionWindow->m_Results.m_LineCheckOk = false;
				}
			}
		}
		else
		{
			ErrorMsg = tr("Can Not Inspect, No Referencedata For Model:%1").arg(pInspectionWindow->m_ModelName);
			return ERROR_CODE_ANY_ERROR;
		}
		
		//QString Path = QString("d://Temp3//lineImage.bmp");// GetImageData()->GetReferenceLocation() + QString("/") + REFERENCE_IMAGE_FILE_NAME;
		//Image.WriteImage("bmp", 0, Path.toLatin1().data());
	}
	catch (HalconCpp::HException &exception)
	{
			ErrorMsg = tr("Error In PrintLineDetector Cut ROI.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			return ERROR_CODE_ANY_ERROR;
	}
	return rv;
}
*/

/*int PrintLineDetector::CalculateTopAndBotLine(HalconCpp::HImage &ROIImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	HalconCpp::HTuple Sigma      = 2.0;
	HalconCpp::HTuple Threshold  = m_ModelParameter.m_MinContrast;//minmale Kantengröße
	HalconCpp::HTuple Transition = "positive";//suche Kante von dunkel nach hell, von unten nach oben
	HalconCpp::HTuple Select     = "first";
	HalconCpp::HTuple RowEdge, ColumnEdge, Amplitude, Distance;
	HalconCpp::HImage ImageRotatet;
	Hlong W, H;


	try
	{
		ROIImage.GetImageSize(&W, &H);
		MeasurePos(ROIImage, m_MeasureHandle, Sigma, Threshold, Transition, Select, &RowEdge, &ColumnEdge, &Amplitude, &Distance);//berechnung unterer Kantenposition der Blockmarke
		pInspectionWindow->m_Results.ClearResults();
		int num = RowEdge.Length();
		if (RowEdge.Length() >= 1)
		{
			double TopLineYpos, BottonLineYpos = RowEdge[0].D();
			double TopLineXpos, BottonLineXpos = ColumnEdge[0].D();
			ImageRotatet = ROIImage.RotateImage(180.0, "bilinear");//bild um 180 Grad drehen und dann untere Kantenposition zu bestimmen
			MeasurePos(ImageRotatet, m_MeasureHandle, Sigma, Threshold, Transition, Select, &RowEdge, &ColumnEdge, &Amplitude, &Distance);
			if (RowEdge.Length() >= 1)
			{
				TopLineYpos = (H - RowEdge[0].D());//Y-Wert Transformieren da Bild um 180 Grad gedreht, dadurch erhält mann die obere Kantenposition der Blockmarke
				TopLineXpos = ColumnEdge[0].D();
				pInspectionWindow->m_Results.m_ResultXPos = (TopLineXpos + BottonLineXpos) / 2.0;//Center pos
				pInspectionWindow->m_Results.m_ResultYPos = (TopLineYpos + BottonLineYpos) / 2.0;//Center Pos
				pInspectionWindow->m_Results.m_ObjectSizeInY = BottonLineYpos - TopLineYpos;//Block height
				pInspectionWindow->m_Results.m_ObjectSizeInX = W;
			}
			else
				pInspectionWindow->m_Results.m_ModelFound = false;
		}
		else
			pInspectionWindow->m_Results.m_ModelFound = false;
		pInspectionWindow->m_Results.m_ResultsValid = true;
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error In PrintLineDetector Calculate Top And Bot Line.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		return ERROR_CODE_ANY_ERROR;
	}
	return rv;
}
*/

/*int PrintLineDetector::GenerateMesureHandle(InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	if (m_ModelParameter.m_ModelWidthInPixel != pInspectionWindow->m_ROIRectRelatetToFormatRect.width() || m_ModelParameter.m_ModelHeightInPixel != pInspectionWindow->m_ROIRectRelatetToFormatRect.height())
	{
		try
		{
			m_ModelParameter.m_ModelWidthInPixel  = pInspectionWindow->m_ROIRectRelatetToFormatRect.width();
			m_ModelParameter.m_ModelHeightInPixel = pInspectionWindow->m_ROIRectRelatetToFormatRect.height();
			HalconCpp::HTuple ImageWidth = m_ModelParameter.m_ModelWidthInPixel;
			HalconCpp::HTuple ImageHeight = m_ModelParameter.m_ModelHeightInPixel;
			HalconCpp::GenMeasureRectangle2(ImageHeight / 2.0, ImageWidth / 2.0, 1.5708, ImageWidth / 2.0, ImageHeight / 2.0, ImageWidth, ImageHeight, "nearest_neighbor", &m_MeasureHandle);
		}
		catch (HalconCpp::HException &exception)
		{
			ErrorMsg = tr("Error In PrintLineDetector Calculate Top And Bot Line.  In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			return ERROR_CODE_ANY_ERROR;
		}
	}
	return ERROR_CODE_NO_ERROR;
}

*/
