#pragma once

#include <QtCore>
#include "halconcpp/HalconCpp.h"
#include "ModelParameter.h"

class MeasurementResult;
class ImageData;
class HoseDetector : QObject
{
public:
	HoseDetector(ImageData *pImageData);
	~HoseDetector();
	ImageData *GetImageData() {	return m_ImageData; }
	int StartDetection(HalconCpp::HImage &InputImage,QString &ErrorMsg);
	int GenerateSyntehticModelHose(QString &ErrorMsg);
	ModelParameter *GetModelParameter(){return &m_ModelParameter;}
	void LoadSettings();
	void SaveSettings();

	//alte variante
	int StartDetection2(HalconCpp::HImage &InputImage, QString &ErrorMsg);
	int GenerateSyntehticModelHose2(QString &ErrorMsg);

private:
	HalconCpp::HTuple  m_ModelID;
	ImageData         *m_ImageData;
	ModelParameter     m_ModelParameter;
};

