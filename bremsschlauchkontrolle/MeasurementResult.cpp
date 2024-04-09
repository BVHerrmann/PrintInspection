#include "MeasurementResult.h"
#include "GlobalConst.h"


MeasurementResult::MeasurementResult()
:m_ModelFound(true)
,m_PrintOk(true)
,m_PositionOk(true)
,m_FormatLenghtOk(true)
,m_FormatFound(true)
,m_BlockWidthOk(true)
,m_BlockHeightOk(true)
,m_LineCheckOk(true)
,m_ModelScore(0.0)
,m_ResultXPos(0.0)
,m_ResultYPos(0.0)
,m_ResultScaleFactorInX(0.0)
,m_ResultScaleFactorInY(0.0)
,m_ObjectSizeInX(0.0)
,m_ObjectSizeInY(0.0)
,m_NumberDefectPixel(0)
,m_ResultHoseDetection(STATE_HOSE_UNDEFINED)
,m_ResultsValid(false)
,m_OffsetY(0.0)
,m_DefectScore(0.0)
,m_LenghtErrorInMM(0.0)
,m_HeightErrorInMM(0.0)
,m_FormatLengthInPix(0.0)
,m_MeanGrayValue(0.0)
{
	
}


MeasurementResult::~MeasurementResult()
{
	ClearResults();
}


void MeasurementResult::ClearResults()
{
	m_ModelFound           = true;
	m_PrintOk              = true;
	m_PositionOk           = true;
	m_FormatFound          = true;
	m_FormatLenghtOk       = true;
	m_BlockWidthOk         = true;
	m_BlockHeightOk        = true;
	m_LineCheckOk          = true;
	m_ResultsValid         = false;
	m_ModelScore           = 0.0;
	m_ResultXPos           = 0.0;
	m_ResultYPos           = 0.0;
	m_ResultScaleFactorInX = 0.0;
	m_ResultScaleFactorInY = 0.0;
	m_ObjectSizeInX        = 0.0;
	m_ObjectSizeInY        = 0.0;
	m_NumberDefectPixel    = 0;
	m_DefectScore          = 0.0;
	m_OffsetY              = 0.0;
	m_LenghtErrorInMM      = 0.0;
	m_HeightErrorInMM      = 0.0;
	m_FormatLengthInPix    = 0.0;
	m_MeanGrayValue        = 0.0;
	
	m_ResultHoseDetection  = STATE_HOSE_UNDEFINED;
}

bool MeasurementResult::IsQualityOk()
{
	if (m_ModelFound && m_FormatFound && m_FormatLenghtOk && m_PositionOk && m_PrintOk && m_BlockWidthOk && m_BlockHeightOk && m_LineCheckOk)
		return true;
	else
		return false;
}



