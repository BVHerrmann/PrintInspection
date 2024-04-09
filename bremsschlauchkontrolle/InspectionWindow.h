#pragma once
#include "qstring.h"
#include "qrect.h"

#include "halconcpp/HalconCpp.h"
#include "MeasurementResult.h"

class InspectionWindow
{
public:
	InspectionWindow()
	{
		m_InspectionWindowID       = 0;
		m_EnableInspection         = true;
		m_CheckOnlyHorizontalLines = false;
		m_HaveReferenceData        = false;
		m_MeasureVarianteDiffImage = true;
		m_NumberHorizontalLines    = 5;//Anzahl der Drucklinen
		m_MaxModelAreaInPixel      = 0;   
		m_ModelName                = "NoName";
		m_ShapeBasedModelID        = 0;
		m_VariationModelID         = 0;
		m_ModelWidthReference      = 0.0;
		m_ModelHeightReference     = 0.0;
		m_ModelXReferencePosition  = 0.0;
		m_ModelYReferencePosition  = 0.0;
		m_ReferenceRect.setX(30.0);
		m_ReferenceRect.setY(480.0);
		m_ReferenceRect.setWidth(400.0);
		m_ReferenceRect.setHeight(200.0);
		m_PrintErrorTolInPercent = 30.0;
	}
	~InspectionWindow()
	{
		if(m_ShapeBasedModelID!=0)
		   ClearShapeModel(m_ShapeBasedModelID);
		if(m_VariationModelID!=0)
		  ClearVariationModel(m_VariationModelID); 
	}
	void ClearResults()
	{
		m_Results.ClearResults();
	}

	bool IsQualityOk()
	{
		return m_Results.IsQualityOk();
	}

public:
	QRectF             m_ReferenceRect;
	QRectF             m_ROIRectRelatetToFormatRect;
	bool               m_EnableInspection;
	bool               m_CheckOnlyHorizontalLines;
	bool               m_HaveReferenceData;
	bool               m_MeasureVarianteDiffImage;
	int                m_NumberHorizontalLines;
	int                m_InspectionWindowID;
	int                m_MaxModelAreaInPixel;
	double             m_ModelWidthReference;
	double             m_ModelHeightReference;
	double             m_ModelXReferencePosition;
	double             m_ModelYReferencePosition;
	double             m_PrintErrorTolInPercent;
	QString            m_ModelName;
	HalconCpp::HTuple  m_ShapeBasedModelID;
	HalconCpp::HTuple  m_VariationModelID;
	MeasurementResult  m_Results;
	HalconCpp::HRegion m_ResultRegionDiff;

	InspectionWindow& operator=(const InspectionWindow& other)
	{
		if (this != &other)
		{
			m_ReferenceRect              = other.m_ReferenceRect;
			m_EnableInspection           = other.m_EnableInspection;
			m_InspectionWindowID         = other.m_InspectionWindowID;
			m_ModelName                  = other.m_ModelName;
			m_ModelWidthReference        = other.m_ModelWidthReference;
			m_ModelHeightReference       = other.m_ModelHeightReference;
			m_ModelXReferencePosition    = other.m_ModelXReferencePosition;
			m_ModelYReferencePosition    = other.m_ModelYReferencePosition;
			m_Results                    = other.m_Results;
			m_MaxModelAreaInPixel        = other.m_MaxModelAreaInPixel;
			m_ROIRectRelatetToFormatRect = other.m_ROIRectRelatetToFormatRect;
			m_CheckOnlyHorizontalLines   = other.m_CheckOnlyHorizontalLines;
			m_NumberHorizontalLines      = other.m_NumberHorizontalLines;
			m_MeasureVarianteDiffImage   = other.m_MeasureVarianteDiffImage;
			m_PrintErrorTolInPercent     = other.m_PrintErrorTolInPercent;
		}
		return *this;
	}
};
