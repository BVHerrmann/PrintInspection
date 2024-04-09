#pragma once

class ModelParameter
{
public:
	double m_ModelWidthInPixel;
	double m_ModelHeightInPixel;
	double m_MaxModelScaleFactor;
	double m_MinModelScaleFactor;
	double m_MaxModelScaleFactorY;
	double m_MinModelScaleFactorY;
	double m_ModelAcceptanceThresholdInPercent;
	double m_SearchAngleRangePosInDegree;
	double m_SearchAngleRangeNegInDegree;
	double m_MinDefectAreaInPixel;
	double m_MinScore;
	double m_ImageScaleFactor;
	double m_GrayThreshold;
	double m_MinContrast;
	double m_ThresholToleranceFactor;
};