#include "WidgetEditReferenceData.h"
#include "GlobalConst.h"
#include "MainAppPrintCheck.h"

#include "WidgetEditReferenceImageData.h"


WidgetEditReferenceData::WidgetEditReferenceData(MainAppPrintCheck *pParent) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)

, m_WidgetEditReferenceCameraTop(NULL)
, m_WidgetEditReferenceCameraBot(NULL)
{
	ui.setupUi(this);
  	m_MainAppPrintCheck = pParent;

	m_WidgetEditReferenceCameraTop = new WidgetEditReferenceImageData(pParent, CAMERA_TOP_INDEX);
	m_WidgetEditReferenceCameraBot = new WidgetEditReferenceImageData(pParent, CAMERA_BOT_INDEX);

	if(ui.frameReferenceDataCameraTop->layout())
	   ui.frameReferenceDataCameraTop->layout()->addWidget((QWidget*)(m_WidgetEditReferenceCameraTop));
	if(ui.frameReferenceDataCameraBot->layout())
	   ui.frameReferenceDataCameraBot->layout()->addWidget((QWidget*)(m_WidgetEditReferenceCameraBot));

}


WidgetEditReferenceData::~WidgetEditReferenceData()
{

}


void WidgetEditReferenceData::ShowSelectedRectKoordinates(int CameraIndex, QRectF &CurrentRect)
{
	
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if(GetWidgetEditReferenceCameraTop())
		   GetWidgetEditReferenceCameraTop()->ShowSelectedRectKoordinates(CurrentRect);
	}
	else
	{
		if(GetWidgetEditReferenceCameraBot())
		  GetWidgetEditReferenceCameraBot()->ShowSelectedRectKoordinates(CurrentRect);
	}
}


void WidgetEditReferenceData::AddReferenceImageWidget(QWidget *w,int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetEditReferenceCameraTop())
			GetWidgetEditReferenceCameraTop()->AddReferenceImageWidget(w);
	}
	else
	{
		if (GetWidgetEditReferenceCameraBot())
			GetWidgetEditReferenceCameraBot()->AddReferenceImageWidget(w); 
	}
}



