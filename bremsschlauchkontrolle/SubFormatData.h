#pragma once
#include <qlist.h>
#include "qrect.h"
#include "MeasurementResult.h"
#include "InspectionWindow.h"


class SubFormatData
{
public:
	SubFormatData();
	~SubFormatData();
	void ClearInspectionWindows();
	int AddNewInspectionWindow(InspectionWindow *pNewWindow);
	void AddNewInspectionWindowDirect(InspectionWindow *NewWindow);
	void RemoveInspectionWindow(int InspectioID);
	int  GetNumberInspectionWindows();
	InspectionWindow *GetInspectionWindowByID(int InspectionWindowID);
	InspectionWindow *GetInspectionWindowByIndex(int Index);
	void SetInspectionWindow(int InspectioWindowID, QRectF &NewRectData);
	int GetIndexByID(int ID);
	InspectionWindow *GetInspectionWindowHoseDetection() { return  &m_InspectionWindowHoseDetection; }

public:
	//parameter
	double m_LenghtWithoutBlockMark;
	double m_LenghtWithBlockMark;
	double m_Height;
	int    m_ProductSide;// TopSide(DOT) 0 o'clock or BottomSide(CCC) 6 o'clock
	//results per format
	double m_ResultInspectionTimeInMs;
	double m_ResultMeanDefectScore;
	double m_ResultMaxDefectScore;
	double m_ResultMeanOffsetInY;
	double m_ResultMaxOffsetInY;
	//MeasurementResult m_ResultsHoseDetection;//includes hose position and hose diameter
	QList<InspectionWindow *> m_ListInspectionWindows;//inspectionwindow inludes paramter and results
	InspectionWindow m_InspectionWindowHoseDetection;
	
};

