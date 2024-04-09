#pragma once
#include "MeasurementResult.h"
#include "InspectionWindow.h"
class VideoHeader
{
public:
	VideoHeader()
	{
		m_ImageWidth             = 0;
		m_ImageHeight            = 0;
		m_ImageBlockSize         = 0;
		m_MaxNumberFrames        = 0;
		m_MAXVideoBockSize       = 0;
		m_CurrentNumberFrames    = 0;
		m_MaxNumberLines         = 0;
		m_StartPosNewFormat      = 0;
		m_FormatEndXpos          = 0;
		m_ImageTimeStampsInMuSec = NULL;
		m_TimeStampFormatImage   = 0;
		m_HoseMiddlePosition     = 0.0;
	}
public:
	int               m_ImageWidth;
	int               m_ImageHeight;
	int               m_ImageBlockSize;
	int               m_FormatEndXpos;
	double            m_HoseMiddlePosition;
	unsigned __int64  m_MaxNumberFrames;
	unsigned __int64  m_MAXVideoBockSize;
	unsigned __int64  m_CurrentNumberFrames;
	unsigned __int64  m_MaxNumberLines;
	unsigned __int64  m_StartPosNewFormat;
	unsigned __int64 *m_ImageTimeStampsInMuSec;//Zeitstempel für die einzelenen Kamerabilder
	unsigned __int64  m_TimeStampFormatImage;//Zeitstempel für ein Formatbild
	//MeasurementResult m_ResultsFormatImage;
	InspectionWindow  m_InspectionWindowFormatResults;
	MeasurementResult m_ResultsHoseDetection;
};