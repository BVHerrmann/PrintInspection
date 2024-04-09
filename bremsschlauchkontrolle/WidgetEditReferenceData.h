#pragma once
#include <qwidget.h>
#include "ui_WidgetEditReferenceData.h"

class ImageMetaData;
class MainAppPrintCheck;
class WidgetEditReferenceImageData;
class WidgetEditReferenceData :	public QWidget
{
	Q_OBJECT
public:
	WidgetEditReferenceData(MainAppPrintCheck *pParent);
	~WidgetEditReferenceData();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	void AddReferenceImageWidget(QWidget *w, int CameraIndex);
	void ShowSelectedRectKoordinates(int CameraIndex, QRectF &rect);
	WidgetEditReferenceImageData *GetWidgetEditReferenceCameraTop() { return m_WidgetEditReferenceCameraTop; }
	WidgetEditReferenceImageData *GetWidgetEditReferenceCameraBot() { return m_WidgetEditReferenceCameraBot; }

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	WidgetEditReferenceImageData *m_WidgetEditReferenceCameraTop;
	WidgetEditReferenceImageData *m_WidgetEditReferenceCameraBot;
	Ui::WidgetEditRefrenceData ui;
};

