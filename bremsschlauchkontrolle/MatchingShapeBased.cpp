#include "MatchingShapeBased.h"
#include "MeasurementResult.h"
#include "GlobalConst.h"
#include "InspectionWindow.h"
#include "ImageData.h"




MatchingShapeBased::MatchingShapeBased(ImageData *pImageData,const QString &MeasureToolName) : QObject(NULL)
	, m_ImageCounter(0)
	, m_ImageData(NULL)
{
	m_MeasureToolName = MeasureToolName;
	m_ImageData       = pImageData;

	if (MeasureToolName == MEASURE_TOOL_NAME_FORMAT_DETECTION)
	{
		m_ModelParameter.m_MaxModelScaleFactor = 1.1;
		m_ModelParameter.m_MinModelScaleFactor = 0.90;
		m_ModelParameter.m_ModelAcceptanceThresholdInPercent = 0.6;// = 60%
		m_ModelParameter.m_SearchAngleRangeNegInDegree = -3.0;
		m_ModelParameter.m_SearchAngleRangePosInDegree = 3.0;
		m_ModelParameter.m_MinDefectAreaInPixel = 200;//wird nicht genutzt
		m_ModelParameter.m_MinScore = 0.5;
		m_ModelParameter.m_ImageScaleFactor = 0.5;
	}
	else
	{//MEASURE_TOOL_NAME_PRINT_CHECK
		m_ModelParameter.m_MaxModelScaleFactor = 1.1;
		m_ModelParameter.m_MinModelScaleFactor = 0.90;
		m_ModelParameter.m_ModelAcceptanceThresholdInPercent = 0.2;
		m_ModelParameter.m_SearchAngleRangeNegInDegree = -3.0;
		m_ModelParameter.m_SearchAngleRangePosInDegree = 3.0;
		m_ModelParameter.m_MinDefectAreaInPixel = 200;//wird nicht genutzt
		m_ModelParameter.m_MinScore = 0.1;
		m_ModelParameter.m_ImageScaleFactor = 1.0;
		m_ModelParameter.m_GrayThreshold = 128;
	}
}


MatchingShapeBased::~MatchingShapeBased()
{
}


void MatchingShapeBased::LoadSettings()
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
		m_ModelParameter.m_MaxModelScaleFactor               = settings.value(QString("PrintCheck%1/%2/MaxModelScaleFactor").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MaxModelScaleFactor).toDouble();
		m_ModelParameter.m_MinModelScaleFactor               = settings.value(QString("PrintCheck%1/%2/MinModelScaleFactor").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinModelScaleFactor).toDouble();
		m_ModelParameter.m_ModelAcceptanceThresholdInPercent = settings.value(QString("PrintCheck%1/%2/ModelAcceptanceThresholdInPercent").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_ModelAcceptanceThresholdInPercent).toDouble();
		m_ModelParameter.m_SearchAngleRangeNegInDegree       = settings.value(QString("PrintCheck%1/%2/SearchAngleRangeNegInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangeNegInDegree).toDouble();
		m_ModelParameter.m_SearchAngleRangePosInDegree       = settings.value(QString("PrintCheck%1/%2/SearchAngleRangePosInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangePosInDegree).toDouble();
		m_ModelParameter.m_MinDefectAreaInPixel              = settings.value(QString("PrintCheck%1/%2/MinNumberDefectsPixel").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinDefectAreaInPixel).toDouble();
		m_ModelParameter.m_MinScore                          = settings.value(QString("PrintCheck%1/%2/MinScoreInPercent").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinScore).toDouble();
		m_ModelParameter.m_GrayThreshold                     = settings.value(QString("PrintCheck%1/%2/GrayThreshold").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_GrayThreshold).toDouble();
		SaveSettings();
	}
}


void MatchingShapeBased::SaveSettings()
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

		settings.setValue(QString("PrintCheck%1/%2/MaxModelScaleFactor").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MaxModelScaleFactor);
		settings.setValue(QString("PrintCheck%1/%2/MinModelScaleFactor").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinModelScaleFactor);
		settings.setValue(QString("PrintCheck%1/%2/ModelAcceptanceThresholdInPercent").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_ModelAcceptanceThresholdInPercent);
		settings.setValue(QString("PrintCheck%1/%2/SearchAngleRangeNegInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangeNegInDegree);
		settings.setValue(QString("PrintCheck%1/%2/SearchAngleRangePosInDegree").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_SearchAngleRangePosInDegree);
		settings.setValue(QString("PrintCheck%1/%2/MinNumberDefectsPixel").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinDefectAreaInPixel);
		settings.setValue(QString("PrintCheck%1/%2/MinScoreInPercent").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_MinScore);
		settings.setValue(QString("PrintCheck%1/%2/GrayThreshold").arg(CameraName).arg(m_MeasureToolName), m_ModelParameter.m_GrayThreshold);

	}
}

//Start der Messung hier wrid entweder nur das Formatgesucht oder die Druckinspektion gestartet
int MatchingShapeBased::StartDetection(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow,QString &ErrorMsg)
{
	if(m_MeasureToolName == MEASURE_TOOL_NAME_FORMAT_DETECTION)
		return StartDetectionFormatCheck(Image,pInspectionWindow,ErrorMsg);
	else
		return StartDetectionPrintCheck(Image, pInspectionWindow, ErrorMsg);
}

//Eigentliche Druckkontrolle. Das Formatbild wird hier übergeben, die entsprechende Bildposition indem sich der zu prüfende Block befindet
int MatchingShapeBased::StartDetectionPrintCheck(HalconCpp::HImage &FormatImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;

	if (pInspectionWindow && pInspectionWindow->m_ShapeBasedModelID != 0)
	{
		HalconCpp::HTuple  W, H, ResultRow, ResultColumn, ResultAngle, ResultScale, ResultScore, ResultScaleR, ResultScaleC;
		HalconCpp::HImage  InspectImage, ROIImage;
		double XTopLeft, YTopLeft, XBottomRight, YBottomRight;
		
		pInspectionWindow->m_Results.ClearResults();
		XTopLeft = YTopLeft = 0.0;
		try
		{//model suchen
			    GetImageSize(FormatImage, &W, &H);
			    int FormatWidth  = (int)W;
			    int FormatHeight = (int)H;
			    XTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().x();
				YTopLeft     = pInspectionWindow->m_ROIRectRelatetToFormatRect.topLeft().y();
				XBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().x() - 1;
				YBottomRight = pInspectionWindow->m_ROIRectRelatetToFormatRect.bottomRight().y() - 1;
				//passt format
				if (XTopLeft < FormatWidth && XBottomRight < FormatWidth)
				{   
					//if (XBottomRight > FormatWidth)
					//	XBottomRight = FormatWidth - 1;//ein Teil des ganz rechts liegenden Messfensters ist nicht mehr enthalten, Bild wird dann reduziert und trotzdem wird versucht eine Inspektion durchzuführen
					try
					{
						ROIImage = FormatImage.CropRectangle1(YTopLeft, XTopLeft, YBottomRight, XBottomRight); //ROI Ausschneiden indem sich der Block befindet
					}
					catch (HalconCpp::HException &exception)
					{
						ErrorMsg = tr("Error Cut ROI. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
						rv= ERROR_CODE_ANY_ERROR;
					}
					if (rv == ERROR_CODE_NO_ERROR)
					   FindAnisoShapeModel(ROIImage, pInspectionWindow->m_ShapeBasedModelID, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinScore, 1, 1, "none", (HalconCpp::HTuple(5).Append(3)), 0.9, &ResultRow, &ResultColumn, &ResultAngle, &ResultScaleR, &ResultScaleC, &ResultScore);
				}
				else
				{//Druck kann nicht durch geführt werden da bild nicht vollständig wahrscheinlich stimmt die gefundene Formatposition nicht
					pInspectionWindow->m_Results.m_ModelFound = false;
					pInspectionWindow->m_Results.m_ModelScore = 0.0;
					pInspectionWindow->m_Results.m_ResultsValid = true;
					//rv = ERROR_CODE_ANY_ERROR;
				}
		}
		catch (HalconCpp::HException &exception)
		{
			ErrorMsg = tr("Error Can Not Detect Model(PrintCheck). In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			rv = ERROR_CODE_ANY_ERROR;
		}
		if (rv == ERROR_CODE_NO_ERROR)
		{
			if (ResultScore.Length() >= 1)
			{//Modell gefunden
				if (ResultScore[0].D() > m_ModelParameter.m_ModelAcceptanceThresholdInPercent)
					pInspectionWindow->m_Results.m_ModelFound = true;
				else
					pInspectionWindow->m_Results.m_ModelFound = false;
				pInspectionWindow->m_Results.m_ModelScore = ResultScore[0].D()  * 100.0;
				pInspectionWindow->m_Results.m_ResultYPos = ResultRow[0].D();//    *ResultScaleR[0].D();
				pInspectionWindow->m_Results.m_ResultXPos = ResultColumn[0].D();// *ResultScaleC[0].D();
				pInspectionWindow->m_Results.m_ResultScaleFactorInX = ResultScaleC[0].D();
				pInspectionWindow->m_Results.m_ResultScaleFactorInY = ResultScaleR[0].D();
				pInspectionWindow->m_Results.m_ModelAngle = ResultAngle[0].D() / RAD_PER_DEGREE;
				pInspectionWindow->m_Results.m_ObjectSizeInX = pInspectionWindow->m_ModelWidthReference  * pInspectionWindow->m_Results.m_ResultScaleFactorInX;// *Results.m_ResultScalFactor;
				pInspectionWindow->m_Results.m_ObjectSizeInY = pInspectionWindow->m_ModelHeightReference * pInspectionWindow->m_Results.m_ResultScaleFactorInY;// *Results.m_ResultScalFactor;
				if (pInspectionWindow->m_VariationModelID != 0 || !pInspectionWindow->m_MeasureVarianteDiffImage)
				{//Blockposition und Orientierung sind jetzt bekannt,hier eigentliche Druckprüfung starten
					rv = StartDetectionVarationModel(ROIImage, pInspectionWindow, ErrorMsg);
				}
				else
				{
					ErrorMsg = tr("Can Not Measure(Varaiation Model) No Model/Product Loaded");
					rv = ERROR_CODE_ANY_ERROR;
				}
				pInspectionWindow->m_Results.m_ResultYPos = pInspectionWindow->m_Results.m_ResultYPos + YTopLeft;
				pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos + XTopLeft;
			}
			else
			{
				pInspectionWindow->m_Results.m_ModelFound = false;
				pInspectionWindow->m_Results.m_ModelScore = 0.0;
			}
		}
	}
	else
	{
		ErrorMsg = tr("Can Not Measure(PrintCheck) No Reference Data");
		rv = ERROR_CODE_ANY_ERROR;
	}
	if (rv == ERROR_CODE_NO_ERROR)
		pInspectionWindow->m_Results.m_ResultsValid = true;
	return rv;
}


int MatchingShapeBased::StartDetectionFormatCheck(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	if (pInspectionWindow && pInspectionWindow->m_ShapeBasedModelID != 0)
	{
		HalconCpp::HTuple  ResultRow, ResultColumn, ResultAngle, ResultScale, ResultScore, ResultScaleR, ResultScaleC;
		HalconCpp::HImage  InspectImage, ROIImage;
		double Score1, Score2;
		int NumMatches;
		int ResultIndex = 0;
		Hlong W, H;
		double XRightPos,XLeftPos;
		int DefaultNumberMatches = 2;//Suche immer nach zwei Formaten im Bild, 
		double Greediness = 0.0;//Gierigkeit der Suchheuristik(0: sicher aber langsam; 1: schnell aber Matches können „übersehen“ werden).
			
		try
		{//model suchen
				if (m_ModelParameter.m_ImageScaleFactor != 1.0)
				{//hier Formatsuche Bild auf Factor 0.5 runterskalieren gilt nur für das Formatbild. Dadurch kann die Formatsuche beschleunigt werden
					HalconCpp::HImage  ScaledImage = Image.ZoomImageFactor(m_ModelParameter.m_ImageScaleFactor, m_ModelParameter.m_ImageScaleFactor, "constant");
					//QString Path = QString("d://Temp3//FormatImage.bmp");
					//ScaledImage.WriteImage("bmp", 0, Path.toLatin1().data());
					FindAnisoShapeModel(ScaledImage, pInspectionWindow->m_ShapeBasedModelID, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinScore, DefaultNumberMatches, 1, "none", (HalconCpp::HTuple(5).Append(3)), Greediness, &ResultRow, &ResultColumn, &ResultAngle, &ResultScaleR, &ResultScaleC, &ResultScore);
				}
				else
					FindAnisoShapeModel(Image, pInspectionWindow->m_ShapeBasedModelID, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, m_ModelParameter.m_MinScore, DefaultNumberMatches, 1, "none", (HalconCpp::HTuple(5).Append(3)), Greediness, &ResultRow, &ResultColumn, &ResultAngle, &ResultScaleR, &ResultScaleC, &ResultScore);
		}
		catch (HalconCpp::HException &exception)
		{//Schwerwiegender Fehler
			ErrorMsg = tr("Error Can Not Detect Model(FormatCheck). In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			return ERROR_CODE_ANY_ERROR;
		}
		pInspectionWindow->m_Results.ClearResults();
		//Hier immer Prüfung ob Format vollständig im Bild, gesucht wird immer nach zwei Formatbildern
    	NumMatches = ResultScore.Length();
		if (NumMatches > 0)
		{
			Image.GetImageSize(&W, &H);
			if (NumMatches == 2)
			{//zwei Model gefunden, Format ist zweimal im Bild enthalten, wähle nur das, welches vollständig ist
				Score1 = ResultScore[0].D();
				Score2 = ResultScore[1].D();
				if (Score1 > m_ModelParameter.m_ModelAcceptanceThresholdInPercent && Score2 > m_ModelParameter.m_ModelAcceptanceThresholdInPercent)
				{//beide Treffer Überschreiten die minmale Schwelle beginne mit dem Topscore an Position 0
					ResultIndex = 0;
					XRightPos = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) + (pInspectionWindow->m_ModelWidthReference / 2.0);
					XLeftPos  = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) - (pInspectionWindow->m_ModelWidthReference / 2.0);
					if (!(XLeftPos >= 0 && XRightPos < W))//ist model vollständig im Bild
					{//nicht vollständig
						ResultIndex = 1;
					    //prüfe zweite Position
						XRightPos = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) + (pInspectionWindow->m_ModelWidthReference / 2.0);
						XLeftPos  = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) - (pInspectionWindow->m_ModelWidthReference / 2.0);
						if (!(XLeftPos >= 0 && XRightPos < W))
						{//nicht vollständig
							ResultIndex = -1;
							pInspectionWindow->m_Results.m_ModelFound = false;
							pInspectionWindow->m_Results.m_ModelScore = 0.0;
						}
     				}
				}
				else
				{//nur eins von beiden überschreitet die Schwelle
					if(Score1 >= Score2)
						ResultIndex = 0;
					else
						ResultIndex = 1;
					XRightPos = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) + (pInspectionWindow->m_ModelWidthReference / 2.0);
					XLeftPos  = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) - (pInspectionWindow->m_ModelWidthReference / 2.0);
					if (!(XLeftPos >= 0 && XRightPos < W))
					{//nicht vollständig
						pInspectionWindow->m_Results.m_ModelFound = false;
						pInspectionWindow->m_Results.m_ModelScore = 0.0;
						ResultIndex = -1;
					}
				}
			}
			else
    		{//ein Model gefunden
				ResultIndex = 0;
				XRightPos = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) + (pInspectionWindow->m_ModelWidthReference / 2.0);
				XLeftPos  = (ResultColumn[ResultIndex].D() / m_ModelParameter.m_ImageScaleFactor) - (pInspectionWindow->m_ModelWidthReference / 2.0);
				if (!(XLeftPos >= 0 && XRightPos < W))
				{//nicht vollständig im Bild enthalten
					pInspectionWindow->m_Results.m_ModelFound = false;
					pInspectionWindow->m_Results.m_ModelScore = 0.0;
					ResultIndex = -1;
				}
	    	}

			if (ResultIndex >= 0)
			{
				if (ResultScore[ResultIndex].D() > m_ModelParameter.m_ModelAcceptanceThresholdInPercent)
					pInspectionWindow->m_Results.m_ModelFound = true;
				else
					pInspectionWindow->m_Results.m_ModelFound = false;
				pInspectionWindow->m_Results.m_ModelScore = ResultScore[ResultIndex].D()  * 100.0;
				pInspectionWindow->m_Results.m_ResultYPos = ResultRow[ResultIndex].D();
				pInspectionWindow->m_Results.m_ResultXPos = ResultColumn[ResultIndex].D();
				pInspectionWindow->m_Results.m_ResultScaleFactorInX = ResultScaleC[ResultIndex].D();
				pInspectionWindow->m_Results.m_ResultScaleFactorInY = ResultScaleR[ResultIndex].D();
				pInspectionWindow->m_Results.m_ModelAngle = ResultAngle[ResultIndex].D() / RAD_PER_DEGREE;
				pInspectionWindow->m_Results.m_ObjectSizeInX = pInspectionWindow->m_ModelWidthReference  * pInspectionWindow->m_Results.m_ResultScaleFactorInX;
				pInspectionWindow->m_Results.m_ObjectSizeInY = pInspectionWindow->m_ModelHeightReference * pInspectionWindow->m_Results.m_ResultScaleFactorInY;
				if (m_ModelParameter.m_ImageScaleFactor != 1.0)
				{
					pInspectionWindow->m_Results.m_ResultXPos = pInspectionWindow->m_Results.m_ResultXPos / m_ModelParameter.m_ImageScaleFactor;
					pInspectionWindow->m_Results.m_ResultYPos = pInspectionWindow->m_Results.m_ResultYPos / m_ModelParameter.m_ImageScaleFactor;
				}
			}
		}
		else
		{
			pInspectionWindow->m_Results.m_ModelFound = false;
			pInspectionWindow->m_Results.m_ModelScore = 0.0;
		}
	}
	else
	{
		ErrorMsg = tr("Can Not Measure(FormatCheck) No Reference Data");
		rv = ERROR_CODE_ANY_ERROR;
	}
	if (rv == ERROR_CODE_NO_ERROR)
		pInspectionWindow->m_Results.m_ResultsValid = true;
	return rv;
}

//Die Blockposition und Orientierung des Block sind bekannt, hier wird der Referenzblock auf den Bildblock gelegt und ein Differenzbild berechnet
int MatchingShapeBased::StartDetectionVarationModel(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow,QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;

	if (GetImageData())
	{
		QString Location = GetImageData()->GetReferenceLocation() + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID);
		QString FilePathAndName;
		HalconCpp::HImage  VarImage, RefImage;
		HalconCpp::HRegion ZoomedRegion, MultiChannelRegion;
		HalconCpp::HImage ROIImage, ImageAffinTrans;
		HalconCpp::HTuple W, H,RGB, HomMat2D, Row(pInspectionWindow->m_Results.m_ResultYPos), Column(pInspectionWindow->m_Results.m_ResultXPos), Angle(pInspectionWindow->m_Results.m_ModelAngle*RAD_PER_DEGREE);
		double ModelWidth2    = pInspectionWindow->m_ModelWidthReference / 2.0;
		double ModelHeight2   = pInspectionWindow->m_ModelHeightReference / 2.0;
		int ROIImageFormatWidth, ROIImageFormatHeight;
		
		

		try
		{	//bild drehen und verschieben auf Referenzposition
			//GetImageSize(Image, &W, &H);
			//int FormatWidth  = (int)W;
			//int FormatHeight = (int)H;
			VectorAngleToRigid(Row, Column, Angle, HalconCpp::HTuple(ModelHeight2), HalconCpp::HTuple(ModelWidth2), 0, &HomMat2D); //transformationsmatrix erzeugen rotation und verschiebung
			AffineTransImage(Image, &ImageAffinTrans, HomMat2D, HalconCpp::HTuple("constant"), HalconCpp::HTuple("false"));//bild drehen und verschieben auf referenzposition
			//Objekt ausschneiden
			ROIImage = ImageAffinTrans.CropPart(0, 0, pInspectionWindow->m_ModelWidthReference, pInspectionWindow->m_ModelHeightReference);//zu untersuchenden bereich herauskopieren
			GetImageSize(ROIImage, &W, &H);
			ROIImageFormatWidth  = (int)W;
			ROIImageFormatHeight = (int)H;

			if (pInspectionWindow->m_ModelWidthReference == ROIImageFormatWidth && pInspectionWindow->m_ModelHeightReference == ROIImageFormatHeight)
			{
				if (pInspectionWindow->m_MeasureVarianteDiffImage)
				{
					CompareExtVariationModel(ROIImage, &(pInspectionWindow->m_ResultRegionDiff), pInspectionWindow->m_VariationModelID, HalconCpp::HTuple("absolute"));
					OpeningRectangle1(pInspectionWindow->m_ResultRegionDiff, &(pInspectionWindow->m_ResultRegionDiff), 3, 3);
				}
				else
				{//einfache Schwellwertmethode, kommt nur zur Anwendung für Blöcke die sich während der Produktion ändern z. B. das Datum
					Threshold(ROIImage, &(pInspectionWindow->m_ResultRegionDiff), m_ModelParameter.m_GrayThreshold, 255);
				}
				//anzahl Fehlstellen ermitteln
				rv = GetNumberBadPixelsAndSetResults(pInspectionWindow->m_ResultRegionDiff, (pInspectionWindow->m_Results.m_NumberDefectPixel), ErrorMsg);
				if (rv == ERROR_CODE_NO_ERROR)
				{
					//HalconCpp::HImage  HRegionBin = pInspectionWindow->m_ResultRegionDiff.RegionToBin(255, 0, (Hlong)(pInspectionWindow->m_ModelWidthReference), (Hlong)(pInspectionWindow->m_ModelHeightReference));
					//QString Location = "d://temp//Diffimage.bmp";//
					//HRegionBin.WriteImage("bmp", 0, Location.toLatin1().data());
					if (pInspectionWindow->m_MaxModelAreaInPixel)
					{//Defectscore ermitteln
						if (pInspectionWindow->m_MeasureVarianteDiffImage)
							pInspectionWindow->m_Results.m_DefectScore = (100.0 / pInspectionWindow->m_MaxModelAreaInPixel)*pInspectionWindow->m_Results.m_NumberDefectPixel;
						else
						{
							pInspectionWindow->m_Results.m_DefectScore = ((pInspectionWindow->m_MaxModelAreaInPixel - pInspectionWindow->m_Results.m_NumberDefectPixel) / (double)(pInspectionWindow->m_MaxModelAreaInPixel))*100.0;
							if (pInspectionWindow->m_Results.m_DefectScore < 0.0)
								pInspectionWindow->m_Results.m_DefectScore = 0.0;
						}
					}
				}
				else
				{
					return rv;
				}
			}
		}
		catch (HalconCpp::HException &exception)
		{
			ErrorMsg = tr("Error Check Variation Model. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			rv = ERROR_CODE_ANY_ERROR;
		}
	}
	return rv;
}


int MatchingShapeBased::GetNumberBadPixelsAndSetResults(HalconCpp::HRegion &RegionDiff,int &DefectArea,QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	HalconCpp::HRegion ConnectedRegion;
	HalconCpp::HTuple Area, Row, Column, NDefects,NumberObjects;
	HalconCpp::HImage Image;
	

	DefectArea = 0;
	try
	{
		Connection(RegionDiff, &ConnectedRegion);
		SelectShape(ConnectedRegion, &ConnectedRegion, HalconCpp::HTuple("area"), HalconCpp::HTuple("and"), HalconCpp::HTuple("min"), HalconCpp::HTuple("max"));
		//SelectShape(ConnectedRegion, &ConnectedRegion, HalconCpp::HTuple("area"), HalconCpp::HTuple("and"), HalconCpp::HTuple(m_ModelParameter.m_MinDefectAreaInPixel), HalconCpp::HTuple(1000000));
		AreaCenter(ConnectedRegion, &Area, &Row, &Column);
		CountObj(ConnectedRegion, &NDefects);
		for(int i = 0; i < NDefects.I(); i++)
			DefectArea = DefectArea + Area[i].I();
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error Can Not Calcualte Bad Regions. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int MatchingShapeBased::GenerateModelReferenceData(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow,QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	HalconCpp::HXLDCont ModelContur;
    HalconCpp::HTuple   W, H,MinY, MinX, MaxY, MaxX, ParameterName, ParameterValueContrast, ParameterValueMinContrast;
	HalconCpp::HRegion ModelRegion,ShapeModelRegions;
	HalconCpp::HImage  ScaledImage,ModelImage, ROIImage;
	QRectF ROIRect = pInspectionWindow->m_ReferenceRect;
	QString ReferenceLocation = GetImageData()->GetReferenceLocation() + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID);
	QString FileNameModelContour = "/" + SHAPE_BASED_MODEL_CONTOURE_FILE_NAME;
	QString FileNameModelID      = "/" + SHAPE_BASED_MODEL_FILE_NAME;
	QString Location,FilePathAndName;
	QString NameRefImage         = "/" + REFERENCE_MODEL_IMAGE_FILE_NAME;
		
	ParameterValueContrast = 10;
	ParameterValueMinContrast = 21;
	try
	{
		if (pInspectionWindow->m_ShapeBasedModelID != 0)
			ClearShapeModel(pInspectionWindow->m_ShapeBasedModelID);
		QDir().mkpath(ReferenceLocation);
		ROIImage = Image.CropRectangle1(ROIRect.y(), ROIRect.x(), ROIRect.y() + ROIRect.height() - 1, ROIRect.x() + ROIRect.width() - 1);
		if (m_MeasureToolName == MEASURE_TOOL_NAME_FORMAT_DETECTION)
		{
			SmoothImage(ROIImage, &ROIImage, "gauss", 4);//verbessert den matchscore
			if (m_ModelParameter.m_ImageScaleFactor != 1.0)
			{
				ScaledImage = ROIImage.ZoomImageFactor(m_ModelParameter.m_ImageScaleFactor, m_ModelParameter.m_ImageScaleFactor, "constant");
				DetermineShapeModelParams(ScaledImage, 1, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "none", "use_polarity", 10, 30, "contrast", &ParameterName, &ParameterValueContrast);
				DetermineShapeModelParams(ScaledImage, 1, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "none", "use_polarity", 10, 30, "min_contrast", &ParameterName, &ParameterValueMinContrast);

				//int Con= ParameterValueContrast[0];
				//int minCon = ParameterValueMinContrast[0];
				InspectShapeModel(ScaledImage, &ModelImage, &ShapeModelRegions, ParameterValueMinContrast, ParameterValueContrast);
				CreateAnisoShapeModel(ScaledImage, "auto", HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", "auto", "use_polarity", ParameterValueContrast, ParameterValueMinContrast, &(pInspectionWindow->m_ShapeBasedModelID));
				GetShapeModelContours(&ModelContur, pInspectionWindow->m_ShapeBasedModelID, 1);
			}
			else
			{
				DetermineShapeModelParams(ROIImage, 1, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "none", "use_polarity", 10, 30, "contrast", &ParameterName, &ParameterValueContrast);
				DetermineShapeModelParams(ROIImage, 1, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "none", "use_polarity", 10, 30, "min_contrast", &ParameterName, &ParameterValueMinContrast);
				InspectShapeModel(ROIImage, &ModelImage, &ShapeModelRegions, ParameterValueMinContrast, ParameterValueContrast);
				CreateAnisoShapeModel(ROIImage, "auto", HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", "auto", "use_polarity", ParameterValueContrast, ParameterValueMinContrast, &(pInspectionWindow->m_ShapeBasedModelID));
				GetShapeModelContours(&ModelContur, pInspectionWindow->m_ShapeBasedModelID, 1);
			}
		}
		else
		{
			DetermineShapeModelParams(ROIImage, 1, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "none", "use_polarity", 10, 30, "contrast", &ParameterName, &ParameterValueContrast);
			DetermineShapeModelParams(ROIImage, 1, HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "none", "use_polarity", 10, 30, "min_contrast", &ParameterName, &ParameterValueMinContrast);
			//SmoothImage(ROIImage, &ROIImage, "gauss", 2);
			InspectShapeModel(ROIImage, &ModelImage, &ShapeModelRegions, ParameterValueMinContrast, ParameterValueContrast);
			CreateAnisoShapeModel(ROIImage, "auto", HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangeNegInDegree).TupleRad(), HalconCpp::HTuple(m_ModelParameter.m_SearchAngleRangePosInDegree).TupleRad(), "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", m_ModelParameter.m_MinModelScaleFactor, m_ModelParameter.m_MaxModelScaleFactor, "auto", "auto", "use_polarity", ParameterValueContrast, ParameterValueMinContrast, &(pInspectionWindow->m_ShapeBasedModelID));
			GetShapeModelContours(&ModelContur, pInspectionWindow->m_ShapeBasedModelID, 1);
		}
		FilePathAndName = ReferenceLocation + FileNameModelContour;
		ShapeModelRegions.WriteRegion(FilePathAndName.toLatin1().data());

		FilePathAndName = ReferenceLocation + FileNameModelID;
		WriteShapeModel(pInspectionWindow->m_ShapeBasedModelID, FilePathAndName.toLatin1().data());//model speichern
		
		SmallestRectangle1(ShapeModelRegions, &MinY, &MinX, &MaxY, &MaxX);
		ROIImage.GetImageSize(&W, &H);
		
		pInspectionWindow->m_ModelWidthReference =  ROIRect.width();// MaxX.D() - MinX.D();
		pInspectionWindow->m_ModelHeightReference = ROIRect.height();// MaxY.D() - MinY.D();

		Location = ReferenceLocation + NameRefImage;
		ROIImage.WriteImage("bmp", 0, Location.toLatin1().data());

		/*if (pInspectionWindow->m_InspectionWindowID == 0)
		{
		  	SmoothImage(ROIImage, &ROIImage, "gauss", 4);
			Location = ReferenceLocation + QString("/SmoothImage.bmp");
			ROIImage.WriteImage("bmp", 0, Location.toLatin1().data());
		}
		*/
		pInspectionWindow->m_HaveReferenceData = true;
		if (m_MeasureToolName == MEASURE_TOOL_NAME_PRINT_CHECK)
		    rv = GenerateVariationModel(ROIImage, pInspectionWindow, ErrorMsg);
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error Can Not Create Model Reference Data. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int MatchingShapeBased::GenerateVariationModel(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow,QString &ErrorMsg)
{
	HalconCpp::HTuple Width, Height;
	HalconCpp::HImage  VarImage, BlankImage, RegionBin;
	HalconCpp::HRegion DiffRegion;
	QString ReferenceLocation     = GetImageData()->GetReferenceLocation() + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID);
	QString NameVariationModel    = "/" + VARIATION_MODEL_FILE_NAME;
	QString	NameVarImage          = "/" + VARIATION_MODEL_IMAGE_FILE_NAME;
	QString Location;
	int rv= ERROR_CODE_NO_ERROR;
	int w, h;

	try
	{
		if (pInspectionWindow->m_VariationModelID != 0)
			ClearVariationModel(pInspectionWindow->m_VariationModelID);
		GetImageSize(Image, &Width, &Height);
		w = (int)Width;
		h = (int)Height;
		rv=GenerateVariationImage(Image, &VarImage, ErrorMsg);//erzeuge variationsbild
		if (rv == ERROR_CODE_NO_ERROR)
		{
			if (pInspectionWindow->m_MeasureVarianteDiffImage)
			{   //erzeuge differenzbild wenn kein objekt im bild 
				CreateVariationModel(Width, Height, HalconCpp::HTuple("byte"), HalconCpp::HTuple("direct"), &(pInspectionWindow->m_VariationModelID));//erzeuge modell
				PrepareDirectVariationModel(((HalconCpp::HObject)(Image)), ((HalconCpp::HObject)(VarImage)), pInspectionWindow->m_VariationModelID, m_ModelParameter.m_GrayThreshold, 1);
				GenImageConst(&BlankImage, HalconCpp::HTuple("byte"), Width, Height);//erzeuge testbild, es ist komplett schwarz
				CompareExtVariationModel(BlankImage, &DiffRegion, pInspectionWindow->m_VariationModelID, HalconCpp::HTuple("absolute"));
			}
			else
			{
				Threshold(Image, &DiffRegion, m_ModelParameter.m_GrayThreshold, 255);
			}
			rv = GetNumberBadPixelsAndSetResults(DiffRegion, (pInspectionWindow->m_MaxModelAreaInPixel), ErrorMsg);//Bestimmung der Fläche des Modells
			if (rv == ERROR_CODE_NO_ERROR)
			{
				Location = ReferenceLocation + NameVariationModel;
				if (pInspectionWindow->m_MeasureVarianteDiffImage)
			        WriteVariationModel(pInspectionWindow->m_VariationModelID, Location.toLatin1().data());//modell speichern
				RegionToBin(DiffRegion, &RegionBin, 255, 0, Width, Height);
				Location = ReferenceLocation + NameVarImage;
				RegionBin.WriteImage("bmp", 0, Location.toLatin1().data());
			}
		}
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error Can Not Generate Variation Model. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int MatchingShapeBased::GenerateVariationImage(HalconCpp::HImage &ROIImage, HalconCpp::HImage *VarImage, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;

	try
	{
		SobelAmp(ROIImage, VarImage, "sum_abs", 5);
	}
	catch (HalconCpp::HException &exception)
	{
		ErrorMsg = tr("Error Can Not Generate Variation Image. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
		rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}

