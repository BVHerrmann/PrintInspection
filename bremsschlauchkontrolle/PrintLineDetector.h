#pragma once
#include <qobject.h>
#include "ModelParameter.h"
#include "halconcpp/HalconCpp.h"

class ImageData;
class InspectionWindow;
class PrintLineDetector : public QObject
{
public:
	PrintLineDetector(ImageData *pImageData, const QString &MeasureToolName);
	~PrintLineDetector();

	int GenerateSyntehticModelRectangle(QString &ErrorMsg);
	
	int  StartDetection(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	int  GenerateModelReferenceData(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	//int  CalculateTopAndBotLine(HalconCpp::HImage &Image, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	//int  GenerateMesureHandle(InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	int  CheckLineMissing(HalconCpp::HImage &ROIImage, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	//int  ReadReferenceBlockSize(InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	void LoadSettings();
	void SaveSettings();
	void ClearModel();

	ModelParameter *GetModelParameter() { return &m_ModelParameter; }
	ImageData      *GetImageData()      { return m_ImageData; }

private:
	HalconCpp::HTuple  m_ModelID;
	HalconCpp::HTuple  m_MeasureHandle;
	ImageData         *m_ImageData;
	ModelParameter     m_ModelParameter;
	int m_ImageCounter;
	QString m_MeasureToolName;
	
};

