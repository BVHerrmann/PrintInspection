#pragma once
//#include "qs"

class MeasurementResultsPerBlock
{
public:
	MeasurementResultsPerBlock()
	{
		ClearResults();
	}
	void ClearResults()
	{
		m_BlockID = 0;
		m_XPos = 0;
		m_YPos = 0;
		m_Width = 0;
		m_Height = 0;
		m_MatchScore = 0.0;
	}
public:
	int m_BlockID;
	int m_XPos;
	int m_YPos;
	int m_Width;
	int m_Height;
	double m_MatchScore;
};