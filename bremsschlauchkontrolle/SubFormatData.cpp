#include "SubFormatData.h"
#include "GlobalConst.h"
#include "QObject.h"



SubFormatData::SubFormatData()
	:m_ResultInspectionTimeInMs(0.0)
	,m_ResultMeanDefectScore(0.0)
	,m_ResultMaxDefectScore(0.0)
	,m_ResultMeanOffsetInY(0.0)
    ,m_ResultMaxOffsetInY(0.0)
{
	m_ListInspectionWindows.append(new InspectionWindow());//Format window
	m_ListInspectionWindows.at(0)->m_ModelName = MODEL_NAME_FORMAT;
	//beim start hat dieses fenster noch keine defaultgröße
	m_ListInspectionWindows.at(0)->m_ReferenceRect.setX(0.0);
	m_ListInspectionWindows.at(0)->m_ReferenceRect.setY(0.0);
	m_ListInspectionWindows.at(0)->m_ReferenceRect.setWidth(0.0);
	m_ListInspectionWindows.at(0)->m_ReferenceRect.setHeight(0.0);

	m_InspectionWindowHoseDetection.m_InspectionWindowID = INSPECTION_ID_HOSE_WINDOW;
	m_InspectionWindowHoseDetection.m_ModelName          = QString("HoseDetection");
	//wird hier als messfenster genutzt
	m_InspectionWindowHoseDetection.m_ReferenceRect.setX(256.0);
	m_InspectionWindowHoseDetection.m_ReferenceRect.setY(0.0);
	m_InspectionWindowHoseDetection.m_ReferenceRect.setWidth(512.0);
	m_InspectionWindowHoseDetection.m_ReferenceRect.setHeight(1024.0);
}


SubFormatData::~SubFormatData()
{
}


void SubFormatData::ClearInspectionWindows()
{
	for (int i = 0; i < m_ListInspectionWindows.count(); i++)
		delete m_ListInspectionWindows.at(i);
    m_ListInspectionWindows.clear();
}


int SubFormatData::GetNumberInspectionWindows()
{
	return m_ListInspectionWindows.count();
}


int SubFormatData::AddNewInspectionWindow(InspectionWindow *NewWindow)
{
	int newID = 1;

	if (m_ListInspectionWindows.count() == 0)
	{
		m_ListInspectionWindows.append(new InspectionWindow());//Master window
		m_ListInspectionWindows.at(0)->m_ModelName = MODEL_NAME_FORMAT;
	}

	for (int i = 0; i < m_ListInspectionWindows.count(); i++)
	{
		if (m_ListInspectionWindows.at(i)->m_InspectionWindowID == newID)
		{
			newID++;
			i = 0;
		}
	}
	NewWindow->m_InspectionWindowID = newID;
	m_ListInspectionWindows.append(NewWindow);
	return newID;
}


void SubFormatData::AddNewInspectionWindowDirect(InspectionWindow *NewWindow)
{
	m_ListInspectionWindows.append(NewWindow);
}


void SubFormatData::RemoveInspectionWindow(int InspectioID)
{
	for (int i = 0; i < m_ListInspectionWindows.count(); i++)
	{
		if (InspectioID == m_ListInspectionWindows.at(i)->m_InspectionWindowID)
		{
			delete m_ListInspectionWindows.at(i);
			m_ListInspectionWindows.removeAt(i);
			break;
		}
	}
}

InspectionWindow *SubFormatData::GetInspectionWindowByID(int InspectioWindowID)
{
	InspectionWindow *pInspectionWindow = NULL;
	for (int i = 0; i < m_ListInspectionWindows.count(); i++)
	{
		if (InspectioWindowID == m_ListInspectionWindows.at(i)->m_InspectionWindowID)
		{
			pInspectionWindow = m_ListInspectionWindows.at(i);
			break;
		}
    }
	if (m_ListInspectionWindows.count() == 0)
	{//eins muss immer drin sein
		pInspectionWindow = new InspectionWindow();
		m_ListInspectionWindows.append(pInspectionWindow);//Format window
		m_ListInspectionWindows.at(0)->m_ModelName = MODEL_NAME_FORMAT;
	}
	return pInspectionWindow;
}


int SubFormatData::GetIndexByID(int ID)
{
	int rv = -1;
	for (int i = 0; i < m_ListInspectionWindows.count(); i++)
	{
		if (ID == m_ListInspectionWindows.at(i)->m_InspectionWindowID)
		{
			rv = i;
			break;
		}
	}
	return rv;
}


InspectionWindow *SubFormatData::GetInspectionWindowByIndex(int Index)
{
	if (Index >= 0 && Index < m_ListInspectionWindows.count())
		return m_ListInspectionWindows.at(Index);
	else
	    return NULL;
}


void SubFormatData::SetInspectionWindow(int InspectioWindowID, QRectF &NewRectData)
{
	InspectionWindow *pInspectionRect = GetInspectionWindowByID(InspectioWindowID);
	if (pInspectionRect)
	{
		pInspectionRect->m_ReferenceRect = NewRectData;
	}
}
