#pragma once
#include <qwidget.h>
#include "ui_WidgetEditReferenceImageData.h"
#include "bmessagebox.h"

class PopupDialogEditCameraROI;
class MainAppPrintCheck;
class WidgetEditReferenceImageData : public QWidget
{
	Q_OBJECT
public:
	WidgetEditReferenceImageData(MainAppPrintCheck *pParent, int CameraIndex);
	~WidgetEditReferenceImageData();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	int GetCameraIndex() { return m_CameraIndex; }
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void AddReferenceImageWidget(QWidget *w);
	void ShowSelectedRectKoordinates(QRectF &rect);
	void ShowInfoBoxStatusGeneraterefData(const QString ErrorMsg);
	PopupDialogEditCameraROI *GetPopupDialogEditCameraROI(){return m_PopupDialogEditCameraROI;}

public slots:
	void SlotCheckNewReference();
	void SlotSaveAllReferenceData();
	void SlotDeleteSelectedRect();
	void SlotAddNewRect();
	void SlotReloadReferenceImage();
	void SlotCenterAllMeasureWidows();
	void SlotShowRuler(int State);

	void SlotROIXPosChanged();
	void SlotROIYPosChanged();
	void SlotROIWidthChanged();
	void SlotROIHeightChanged();

private:
	MainAppPrintCheck               *m_MainAppPrintCheck;
	int                              m_CameraIndex;
	bool                             m_WindowSetup;
	Ui::WidgetEditReferenceImageData ui;
	BMessageBox                     *m_MessageBox;
	PopupDialogEditCameraROI        *m_PopupDialogEditCameraROI;
};

