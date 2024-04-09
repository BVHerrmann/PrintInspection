#include "ProductData.h"
#include "MainAppPrintCheck.h"
#include "GlobalConst.h"
#include "InspectionWindow.h"


QDataStream &operator<<(QDataStream &ds, ProductData *pProductData)
{
	ds << CURRENT_FILE_VERSION_NUMBER;
	ds << pProductData->m_ProductID;
	ds << pProductData->m_ProductDiameter;
	ds << pProductData->m_BlockWidthTolInMM;
	ds << pProductData->m_BlockHeightTolInMM;
	ds << pProductData->m_PrintErrorTolInPercent;
	ds << pProductData->m_ProductName;
	ds << pProductData->m_PositionTolInMM;
	ds << pProductData->m_FormatLenghtTolInMM;
	ds << pProductData->m_CameraTopType;
	ds << pProductData->m_CameraBotType;
	ds << pProductData->m_FormatLenghtInMMTopCamera;
	ds << pProductData->m_FormatLenghtInMMBotCamera;
	ds << pProductData->m_NumberEvaluationPeriodFormatNotFound;
	
	for (int cam = 0; cam < NUMBER_CAMERAS; cam++)
	{
		ds << pProductData->GetNumberMeasureWindows(cam);
		SubFormatData *pSubFormatData=pProductData->GetSubFormat(cam);
		if (pSubFormatData)
		{
			for (int i = 0; i < pProductData->GetNumberMeasureWindows(cam); i++)
			{
				InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowByIndex(i);
				if (pInspectionWindow)
				{
					ds << pInspectionWindow->m_InspectionWindowID;
					ds << pInspectionWindow->m_ModelName;
					ds << pInspectionWindow->m_EnableInspection;

					ds << pInspectionWindow->m_ReferenceRect.x();
					ds << pInspectionWindow->m_ReferenceRect.y();
					ds << pInspectionWindow->m_ReferenceRect.width();
					ds << pInspectionWindow->m_ReferenceRect.height();

					ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.x();
					ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.y();
					ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.width();
					ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.height();

					ds << pInspectionWindow->m_ModelWidthReference;
					ds << pInspectionWindow->m_ModelHeightReference;
					ds << pInspectionWindow->m_ModelXReferencePosition;
					ds << pInspectionWindow->m_ModelYReferencePosition;
					ds << pInspectionWindow->m_MaxModelAreaInPixel;
					ds << pInspectionWindow->m_MeasureVarianteDiffImage;

					ds << pInspectionWindow->m_CheckOnlyHorizontalLines;
					ds << pInspectionWindow->m_NumberHorizontalLines;

					ds << pInspectionWindow->m_PrintErrorTolInPercent;
				}
			}
			InspectionWindow *pInspectionWindow = pSubFormatData->GetInspectionWindowHoseDetection();
			if (pInspectionWindow)
			{
				ds << pInspectionWindow->m_InspectionWindowID;
				ds << pInspectionWindow->m_ModelName;
				ds << pInspectionWindow->m_ReferenceRect.x();
				ds << pInspectionWindow->m_ReferenceRect.y();
				ds << pInspectionWindow->m_ReferenceRect.width();
				ds << pInspectionWindow->m_ReferenceRect.height();

				ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.x();
				ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.y();
				ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.width();
				ds << pInspectionWindow->m_ROIRectRelatetToFormatRect.height();

				ds << pInspectionWindow->m_ModelWidthReference;
				ds << pInspectionWindow->m_ModelHeightReference;
				ds << pInspectionWindow->m_ModelXReferencePosition;
				ds << pInspectionWindow->m_ModelYReferencePosition;
				ds << pInspectionWindow->m_MaxModelAreaInPixel;
				ds << pInspectionWindow->m_MeasureVarianteDiffImage;
			}
		}
	}
	
	return ds;
}


QDataStream &operator>>(QDataStream &ds, ProductData *pProductData)
{
	int NumberMeasureWindows;
	int header=0;
	double x, y, w, h;
	QString ErrorMsg;

	if (!ds.atEnd())
		ds >> header;
	if (header < FILE_VERSION_BASE_NUMBER)//check exist header
	{
		pProductData->m_ProductID = header;
		pProductData->m_FileVersionNumber = FILE_VERSION_BASE_NUMBER;
	}
	else
	{
		pProductData->m_FileVersionNumber = header;
		if (!ds.atEnd())
			ds >> pProductData->m_ProductID;
	}
	if (!ds.atEnd())
		ds >> pProductData->m_ProductDiameter;
	if (!ds.atEnd())
		ds >> pProductData->m_BlockWidthTolInMM;
	if (!ds.atEnd())
		ds >> pProductData->m_BlockHeightTolInMM;
	if (!ds.atEnd())
		ds >> pProductData->m_PrintErrorTolInPercent;
	if (!ds.atEnd())
		ds >> pProductData->m_ProductName;
	if (!ds.atEnd())
	    ds >> pProductData->m_PositionTolInMM;
	if (!ds.atEnd())
	    ds >> pProductData->m_FormatLenghtTolInMM;
	if (!ds.atEnd())
	    ds >> pProductData->m_CameraTopType;
	if (!ds.atEnd())
	   ds >> pProductData->m_CameraBotType;
	if (!ds.atEnd())
	   ds >> pProductData->m_FormatLenghtInMMTopCamera;
	if (!ds.atEnd())
	   ds >> pProductData->m_FormatLenghtInMMBotCamera;
	if (!ds.atEnd())
	   ds >> pProductData->m_NumberEvaluationPeriodFormatNotFound;
	
	if (!ds.atEnd())
	{
		for (int cam = 0; cam < NUMBER_CAMERAS; cam++)
		{
			ds >> NumberMeasureWindows;
			pProductData->ClearInspectionWindows(cam);
			for (int i = 0; i < NumberMeasureWindows; i++)
			{
				InspectionWindow *pInspectionWindow = new InspectionWindow();// pProductData->GetSubFormat(cam)->GetInspectionWindowByIndex(i);
				if (pInspectionWindow)
				{
					ds >> pInspectionWindow->m_InspectionWindowID;
					ds >> pInspectionWindow->m_ModelName;

					if(pProductData->m_FileVersionNumber >= 1001)//nachträglich eingefügt
						ds >> pInspectionWindow->m_EnableInspection;

					ds >> x;
					ds >> y;
					ds >> w;
					ds >> h;
					pInspectionWindow->m_ReferenceRect.setX(x);
					pInspectionWindow->m_ReferenceRect.setY(y);
					pInspectionWindow->m_ReferenceRect.setWidth(w);
					pInspectionWindow->m_ReferenceRect.setHeight(h);

					ds >> x;
					ds >> y;
					ds >> w;
					ds >> h;
					pInspectionWindow->m_ROIRectRelatetToFormatRect.setX(x);
					pInspectionWindow->m_ROIRectRelatetToFormatRect.setY(y);
					pInspectionWindow->m_ROIRectRelatetToFormatRect.setWidth(w);
					pInspectionWindow->m_ROIRectRelatetToFormatRect.setHeight(h);


					ds >> pInspectionWindow->m_ModelWidthReference;
					ds >> pInspectionWindow->m_ModelHeightReference;
					ds >> pInspectionWindow->m_ModelXReferencePosition;
					ds >> pInspectionWindow->m_ModelYReferencePosition;
					ds >> pInspectionWindow->m_MaxModelAreaInPixel;
					ds >> pInspectionWindow->m_MeasureVarianteDiffImage;

					ds >> pInspectionWindow->m_CheckOnlyHorizontalLines;
					ds >> pInspectionWindow->m_NumberHorizontalLines;
					if (pProductData->m_FileVersionNumber <= 1001)
						pInspectionWindow->m_PrintErrorTolInPercent = pProductData->m_PrintErrorTolInPercent;
					else
						ds >> pInspectionWindow->m_PrintErrorTolInPercent;//Schwellwert für jeden Messblock
				}
				pProductData->ReadModelData(cam, pInspectionWindow, ErrorMsg);
				pProductData->GetSubFormat(cam)->m_ListInspectionWindows.append(pInspectionWindow);
			}
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_InspectionWindowID;
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ModelName;

			ds >> x;
			ds >> y;
			ds >> w;
			ds >> h;
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ReferenceRect.setX(x);
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ReferenceRect.setY(y);
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ReferenceRect.setWidth(w);
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ReferenceRect.setHeight(h);

			ds >> x;
			ds >> y;
			ds >> w;
			ds >> h;
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ROIRectRelatetToFormatRect.setX(x);
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ROIRectRelatetToFormatRect.setY(y);
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ROIRectRelatetToFormatRect.setWidth(w);
			pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ROIRectRelatetToFormatRect.setHeight(h);


			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ModelWidthReference;
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ModelHeightReference;
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ModelXReferencePosition;
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_ModelYReferencePosition;
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_MaxModelAreaInPixel;
			ds >> pProductData->GetSubFormat(cam)->m_InspectionWindowHoseDetection.m_MeasureVarianteDiffImage;
		}
	}
	return ds;
}


ProductData::ProductData(MainAppPrintCheck *pParent, QString &ProductName) : QObject()
, m_ProductID(0)//job id for PLC
, m_ProductDiameter(10.0)//in mm
, m_PrintErrorTolInPercent(30.0)//in percent
, m_BlockHeightTolInMM(1.0)//+/- mm
, m_BlockWidthTolInMM(3.0)//+/- mm
, m_ProductName(QString("Default"))
, m_MainAppPrintCheck(NULL)
, m_PositionTolInMM(3.0)//+/- mm
, m_FormatLenghtTolInMM(3.0)//+/- mm
, m_CameraTopType("CCC")
, m_CameraBotType("DOT")
, m_FormatLenghtInMMTopCamera(138.0)
, m_FormatLenghtInMMBotCamera(83.0)
, m_NumberEvaluationPeriodFormatNotFound(2)
, m_FileVersionNumber(1)
{
	m_MainAppPrintCheck = pParent;
	SetNewProductLocationAndName(ProductName);
	m_SubFormatDataCamera[CAMERA_TOP_INDEX].m_ProductSide = CAMERA_TOP_INDEX;
	m_SubFormatDataCamera[CAMERA_BOT_INDEX].m_ProductSide = CAMERA_BOT_INDEX;

	int W = m_MainAppPrintCheck->GetCameraWidth(CAMERA_TOP_INDEX);
	int H = m_MainAppPrintCheck->GetCameraHeight(CAMERA_TOP_INDEX);
	m_SubFormatDataCamera[CAMERA_TOP_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setX(W / 4.0);
	m_SubFormatDataCamera[CAMERA_TOP_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setY(0.0);
	m_SubFormatDataCamera[CAMERA_TOP_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setWidth(W / 2.0);
	m_SubFormatDataCamera[CAMERA_TOP_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setHeight(H);

	W = m_MainAppPrintCheck->GetCameraWidth(CAMERA_BOT_INDEX);
	H = m_MainAppPrintCheck->GetCameraHeight(CAMERA_BOT_INDEX);
	m_SubFormatDataCamera[CAMERA_BOT_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setX(W / 4.0);
	m_SubFormatDataCamera[CAMERA_BOT_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setY(0.0);
	m_SubFormatDataCamera[CAMERA_BOT_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setWidth(W/2.0);
	m_SubFormatDataCamera[CAMERA_BOT_INDEX].m_InspectionWindowHoseDetection.m_ReferenceRect.setHeight(H);
}


ProductData::~ProductData()
{
	for (int cam = 0; cam < NUMBER_CAMERAS; cam++)
	{
		ClearInspectionWindows(cam);
	}
}


void ProductData::SetNewProductLocationAndName(QString &NewProductName)
{
	QDir MakeDir;
	m_ProductName = NewProductName;
	m_LocationProductData = GetMainAppPrintCheck()->GetProductLocation() + QString("/") + m_ProductName;
	MakeDir.mkdir(m_LocationProductData);

	m_LocationCameraTopData = m_LocationProductData + QString("/%1").arg(CAMERA_TOP_DIR_NAME);
	MakeDir.mkdir(m_LocationCameraTopData);
	m_LocationCameraBotData = m_LocationProductData + QString("/%1").arg(CAMERA_BOT_DIR_NAME);
	MakeDir.mkdir(m_LocationCameraBotData);
}


int ProductData::GetNumberMeasureWindows(int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX || CameraIndex == CAMERA_BOT_INDEX)
		return m_SubFormatDataCamera[CameraIndex].GetNumberInspectionWindows();
	else
		return 0;
}


SubFormatData * ProductData::GetSubFormat(int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX || CameraIndex == CAMERA_BOT_INDEX)
		return &(m_SubFormatDataCamera[CameraIndex]);
	else
		return NULL;
}


void ProductData::ClearInspectionWindows(int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX || CameraIndex == CAMERA_BOT_INDEX)
	    m_SubFormatDataCamera[CameraIndex].ClearInspectionWindows();
}


void ProductData::AddNewInspectionWindow(int CameraIndex, InspectionWindow *pInspectionWindow)
{
	if (CameraIndex == CAMERA_TOP_INDEX || CameraIndex == CAMERA_BOT_INDEX)
	    m_SubFormatDataCamera[CameraIndex].AddNewInspectionWindow(pInspectionWindow);
}


void ProductData::AddNewInspectionWindowDirect(int CameraIndex, InspectionWindow *pInspectionWindow)
{
	if (CameraIndex == CAMERA_TOP_INDEX || CameraIndex == CAMERA_BOT_INDEX)
		m_SubFormatDataCamera[CameraIndex].AddNewInspectionWindowDirect(pInspectionWindow);
}


int ProductData::ReadProductData(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	QString NameAndPathProductFile =  m_LocationProductData + QString("/") + m_ProductName + QString(".dat"); 
	QFile File(NameAndPathProductFile);
	QFileInfo fileInfo(NameAndPathProductFile);


	if (File.open(QFile::ReadWrite))
	{
			QDataStream in(&File);

			in >> this;
			File.close();
	}
	else
	{
			ErrorMsg = tr("Can Not Open File(Read Product Data) %1").arg(NameAndPathProductFile);
			rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}


int ProductData::ReadModelData(int CameraIndex, InspectionWindow *pInspectionWindow, QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	if (pInspectionWindow)
	{ 
		QString Location, ReferenceLocation;
		QString FileNameModelID = "/" + SHAPE_BASED_MODEL_FILE_NAME;
		QFile ModelFile;
		if (CameraIndex == CAMERA_TOP_INDEX)
			Location = GetLocationCameraTopData();
		else
			Location = GetLocationCameraBotData();
		Location          = Location + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID);
		ReferenceLocation = Location + FileNameModelID;
		ModelFile.setFileName(ReferenceLocation);
		try
		{
			if (ModelFile.exists())
			{
				if (pInspectionWindow->m_ShapeBasedModelID != 0)
					ClearShapeModel(pInspectionWindow->m_ShapeBasedModelID);
				ReadShapeModel(ReferenceLocation.toLatin1().data(), &(pInspectionWindow->m_ShapeBasedModelID));//model laden
			}
			FileNameModelID = "/" + VARIATION_MODEL_FILE_NAME;
			ReferenceLocation = Location + FileNameModelID;
			ModelFile.setFileName(ReferenceLocation);
			if (ModelFile.exists())
			{
				if (pInspectionWindow->m_VariationModelID != 0)
					ClearVariationModel(pInspectionWindow->m_VariationModelID);
				ReadVariationModel(ReferenceLocation.toLatin1().data(), &(pInspectionWindow->m_VariationModelID));
			}
			pInspectionWindow->m_HaveReferenceData = true;
		}
		catch(HalconCpp::HException &exception)
		{//schwerwiegender Fehler
			ErrorMsg = tr("Error/License Error . In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			rv = ERROR_CODE_ANY_ERROR;
		}
    }
	return rv;
}



int  ProductData::WriteProductData(QString &ErrorMsg)
{
	int rv = ERROR_CODE_NO_ERROR;
	
	QString NameAndPathProductFile = m_LocationProductData  + QString("/") + m_ProductName + QString(".dat");
	QFile File(NameAndPathProductFile);
	
	if (File.open(QFile::ReadWrite))
	{
			QDataStream out(&File);

			out << this;
			File.close();
	}
	else
	{
			ErrorMsg = tr("Can Not Open File(Write Product Data) %1").arg(NameAndPathProductFile);
			rv = ERROR_CODE_ANY_ERROR;
	}
	return rv;
}
