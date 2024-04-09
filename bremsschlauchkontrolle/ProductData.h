#pragma once
#include "SubFormatData.h"
#include <QtCore>
#include "GlobalConst.h"
#include "InspectionWindow.h"


class MainAppPrintCheck;
class ProductData : QObject
{
	Q_OBJECT
public:
	ProductData(MainAppPrintCheck *parent,QString &ProductName);
	~ProductData();
	int  WriteProductData(QString &ErrorMsg);
	int  ReadProductData(QString &ErrorMsg);
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	SubFormatData *GetSubFormat(int CameraIndex);
	int GetNumberMeasureWindows(int CameraIndex);
	void ClearInspectionWindows(int CameraIndex);
	QString GetProductName() {return m_ProductName;}
	void SetProductName(QString &set) { m_ProductName = set; }
	void AddNewInspectionWindow(int CameraIndex, InspectionWindow *pInspectionWindow);
	void AddNewInspectionWindowDirect(int CameraIndex, InspectionWindow *pInspectionWindow);
	QString GetLocationCameraTopData() { return m_LocationCameraTopData; }
	QString GetLocationCameraBotData() { return m_LocationCameraBotData; }
	QString GetLocationProductData()   { return m_LocationProductData; }
	//double GetPrintErrorTolInPercent() { return m_PrintErrorTolInPercent; }
	double GetPositionTolInMM()        { return m_PositionTolInMM; }
	double GetFormatLenghtTolInMM()    { return m_FormatLenghtTolInMM; }
	double GetBlockHeightTolInMM()     { return m_BlockHeightTolInMM;}
	double GetBlockWidthTolInMM()      { return m_BlockWidthTolInMM;}
	int ReadModelData(int CameraIndex, InspectionWindow *pInspectionWindow, QString &ErrorMsg);
	void SetNewProductLocationAndName(QString &NewLocation);
	 

public:
	int    m_ProductID;
	int    m_NumberEvaluationPeriodFormatNotFound;
	int    m_FileVersionNumber;
	double m_ProductDiameter;
	double m_PrintErrorTolInPercent;
	double m_PositionTolInMM;
	double m_FormatLenghtTolInMM;
	double m_BlockHeightTolInMM;
	double m_BlockWidthTolInMM;
	double m_FormatLenghtInMMTopCamera;
	double m_FormatLenghtInMMBotCamera;
	QString m_ProductName;
	QString m_CameraTopType;
	QString m_CameraBotType;
	QString m_LocationCameraTopData;
	QString m_LocationCameraBotData;
	QString m_LocationProductData;
	SubFormatData m_SubFormatDataCamera[NUMBER_CAMERAS];
	MainAppPrintCheck *m_MainAppPrintCheck;

	ProductData& operator=(ProductData& other)
	{
		if (this != &other)
		{
			m_ProductDiameter                      = other.m_ProductDiameter;
			m_ProductName                          = other.m_ProductName;
			m_PrintErrorTolInPercent               = other.m_PrintErrorTolInPercent;
			m_PositionTolInMM                      = other.m_PositionTolInMM;
			m_FormatLenghtTolInMM                  = other.m_FormatLenghtTolInMM;
			m_CameraTopType                        = other.m_CameraTopType;
			m_CameraBotType                        = other.m_CameraBotType;
			m_BlockHeightTolInMM                   = other.m_BlockHeightTolInMM;
			m_BlockWidthTolInMM                    = other.m_BlockWidthTolInMM;
			m_FormatLenghtInMMTopCamera            = other.m_FormatLenghtInMMTopCamera;
			m_FormatLenghtInMMBotCamera            = other.m_FormatLenghtInMMBotCamera;
			m_NumberEvaluationPeriodFormatNotFound = other.m_NumberEvaluationPeriodFormatNotFound;

			for (int cam = 0; cam < NUMBER_CAMERAS; cam++)
			{
				ClearInspectionWindows(cam);
				for (int i = 0; i < other.GetSubFormat(cam)->GetNumberInspectionWindows(); i++)
				{
					InspectionWindow *pInspectionWindow = other.GetSubFormat(cam)->GetInspectionWindowByIndex(i);
					if (pInspectionWindow)
					{
						InspectionWindow *NewWindow = new InspectionWindow();
						*NewWindow = *pInspectionWindow;
						AddNewInspectionWindowDirect(cam, NewWindow);
					}
				}
			}
		}
		return *this;
	}
};

