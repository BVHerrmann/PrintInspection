#pragma once
#include "halconcpp/HalconCpp.h"
#include "qobject.h"
#include "ModelParameter.h"



class ImageData;
class InspectionWindow;
class MeasureTaskPrintCheck;
class MatchingShapeBased : QObject
{
public:
	MatchingShapeBased(ImageData *pImageData, const QString &MeasureToolName);
	~MatchingShapeBased();
	
	ImageData *GetImageData() { return m_ImageData; }

	int StartDetection(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow,QString &ErrorMsg);
	int StartDetectionVarationModel(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	int GenerateModelReferenceData(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	int GenerateVariationModel(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow,QString &ErrorMsg);
	int GenerateVariationImage(HalconCpp::HImage &ROIImage, HalconCpp::HImage *VarImage, QString &ErrorMsg);
	int GetNumberBadPixelsAndSetResults(HalconCpp::HRegion &RegionDiff,int &DefectArea, QString &ErrorMsg);

	int StartDetectionPrintCheck(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	int StartDetectionFormatCheck(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);

	ModelParameter  *GetModelParameter() { return &m_ModelParameter; }
	void LoadSettings();
	void SaveSettings();

private:
	ImageData         *m_ImageData;
	int                m_ImageCounter;
	ModelParameter     m_ModelParameter;
	QString            m_MeasureToolName;
};
