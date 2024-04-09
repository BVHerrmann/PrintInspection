#pragma once
#include "qlist.h"

class MeasurementResult
{
public:
	MeasurementResult();
	~MeasurementResult();
	void ClearResults();
	bool IsQualityOk();
	
public:
	bool    m_ModelFound;
	bool    m_PrintOk;
	bool    m_PositionOk;
	bool    m_FormatFound;
	bool    m_FormatLenghtOk;
	bool    m_BlockWidthOk;
	bool    m_BlockHeightOk;
	bool    m_LineCheckOk;
	bool    m_ResultsValid;
	int     m_NumberDefectPixel;
	int     m_ResultHoseDetection;
	double  m_ModelScore; 
	double  m_ResultXPos;
	double  m_ResultYPos;
	double  m_ResultScaleFactorInY;
	double  m_ResultScaleFactorInX;
	double  m_ObjectSizeInX;
	double  m_ObjectSizeInY;
	double  m_ModelAngle;
	double  m_DefectScore;
	double  m_OffsetY;
	double  m_InspectionTimeInms;
	double  m_LenghtErrorInMM;
	double  m_HeightErrorInMM;
	double  m_FormatLengthInPix;
	double  m_MeanGrayValue;
	
	QString m_Date;
	QString m_Time;
	MeasurementResult& operator=(const MeasurementResult& other)
	{
		if (this != &other)
		{
			m_ModelFound              = other.m_ModelFound;
			m_ModelScore              = other.m_ModelScore;
			m_ResultXPos              = other.m_ResultXPos;
			m_ResultYPos              = other.m_ResultYPos;
			m_ResultScaleFactorInX    = other.m_ResultScaleFactorInX;
			m_ResultScaleFactorInY    = other.m_ResultScaleFactorInY;
			m_ObjectSizeInX           = other.m_ObjectSizeInX;
			m_ObjectSizeInY           = other.m_ObjectSizeInY;
			m_ModelAngle              = other.m_ModelAngle;
			m_NumberDefectPixel       = other.m_NumberDefectPixel;
			m_DefectScore             = other.m_DefectScore;
			m_PrintOk                 = other.m_PrintOk;
			m_PositionOk              = other.m_PositionOk;
			m_FormatFound             = other.m_FormatFound;
			m_FormatLenghtOk          = other.m_FormatLenghtOk;
			m_BlockWidthOk            = other.m_BlockWidthOk;
			m_BlockHeightOk           = other.m_BlockHeightOk;
			m_ResultHoseDetection     = other.m_ResultHoseDetection;
			m_ResultsValid            = other.m_ResultsValid;
			m_OffsetY                 = other.m_OffsetY;
			m_InspectionTimeInms      = other.m_InspectionTimeInms;
			m_Date                    = other.m_Date;
			m_Time                    = other.m_Time;
			m_LenghtErrorInMM         = other.m_LenghtErrorInMM;
		    m_HeightErrorInMM         = other.m_HeightErrorInMM;
			m_LineCheckOk             = other.m_LineCheckOk;
			m_FormatLengthInPix       = other.m_FormatLengthInPix;
			m_MeanGrayValue           = other.m_MeanGrayValue;
		
    	}
		return *this;
	}
};

