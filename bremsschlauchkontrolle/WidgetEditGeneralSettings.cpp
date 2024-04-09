#include "WidgetEditGeneralSettings.h"
#include "MainAppPrintCheck.h"
#include "WidgetEditMeasureParameter.h"



WidgetEditGeneralSettings::WidgetEditGeneralSettings(MainAppPrintCheck *pParent) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pParent;

	connect(ui.pushButtonApplySettings,        &QPushButton::clicked, this, &WidgetEditGeneralSettings::SlotApplyGeneralSettings);
	connect(ui.pushButtonApplyNewPixelSizeTop, &QPushButton::clicked, this, &WidgetEditGeneralSettings::SlotApplyPixelSizeTop);
	connect(ui.pushButtonApplyNewPixelSizeBot, &QPushButton::clicked, this, &WidgetEditGeneralSettings::SlotApplyPixelSizeBot);
	
	ui.groupBoxGeneralSettings->setProperty(kRequiredAccessLevel, kAccessLevelAdmin);
}


WidgetEditGeneralSettings::~WidgetEditGeneralSettings()
{
}


void WidgetEditGeneralSettings::ShowCalculatetPixelSize(double PixelSize, double Diameter, int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		ui.doubleSpinBoxMeasuredPixelSizeTop->setValue(PixelSize);
		ui.doubleSpinBoxMeasureDiameterTop->setValue(Diameter);
	}
	else
	{
		ui.doubleSpinBoxMeasuredPixelSizeBot->setValue(PixelSize);
		ui.doubleSpinBoxMeasureDiameterBot->setValue(Diameter);
	}
}


void WidgetEditGeneralSettings::showEvent(QShowEvent *)
{
	if (GetMainAppPrintCheck())
	{
		ui.doubleSpinBoxUsedPixelSizeTop->setValue(GetMainAppPrintCheck()->GetPixelSizeCameraTopInMMPerPixel());
		ui.doubleSpinBoxUsedPixelSizeBot->setValue(GetMainAppPrintCheck()->GetPixelSizeCameraBotInMMPerPixel());

		ui.doubleSpinBoxCameraCenterOffset->setValue(GetMainAppPrintCheck()->GetCameraCenterOffsetInMM());
		ui.doubleSpinBoxTransmissionDistanceCameraTopPLC->setValue(GetMainAppPrintCheck()->GetTransmissionDistanceCameraTopPLCInMM());
		ui.doubleSpinBoxTransmissionDistanceCameraBotPLC->setValue(GetMainAppPrintCheck()->GetTransmissionDistanceCameraBotPLCInMM());
		if (GetMainAppPrintCheck()->GetTopCameraIsFirst())
		{
			ui.radioButtonBotCameraFirst->setChecked(false);
			ui.radioButtonTopCameraFirst->setChecked(true);
		}
		else
		{
			ui.radioButtonBotCameraFirst->setChecked(true);
			ui.radioButtonTopCameraFirst->setChecked(false);
		}
		ui.doubleSpinBoxCameraBotMaxImageHoseNotFound->setValue(GetMainAppPrintCheck()->GetMaxNumberCameraBotImagesHoseNotFound());
		ui.doubleSpinBoxCameraTopMaxImageHoseNotFound->setValue(GetMainAppPrintCheck()->GetMaxNumberCameraTopImagesHoseNotFound());
		ui.doubleSpinBoxCameraBotWaitTimeAfterTubeEnd->setValue(GetMainAppPrintCheck()->GetWaitTimeAfterTubEndCameraBotInms());
		ui.doubleSpinBoxCameraTopWaitTimeAfterTubeEnd->setValue(GetMainAppPrintCheck()->GetWaitTimeAfterTubEndCameraTopInms());
	}
}


void WidgetEditGeneralSettings::SlotApplyGeneralSettings()
{
	if (GetMainAppPrintCheck())
	{
		GetMainAppPrintCheck()->SetCameraCenterOffsetInMM(ui.doubleSpinBoxCameraCenterOffset->value());
		GetMainAppPrintCheck()->SetTransmissionDistanceCameraTopPLCInMM(ui.doubleSpinBoxTransmissionDistanceCameraTopPLC->value());
		GetMainAppPrintCheck()->SetTransmissionDistanceCameraBotPLCInMM(ui.doubleSpinBoxTransmissionDistanceCameraBotPLC->value());
	
		if(ui.radioButtonTopCameraFirst->isChecked())
			GetMainAppPrintCheck()->SetTopCameraIsFirst(true);
		else
			GetMainAppPrintCheck()->SetTopCameraIsFirst(false);

		GetMainAppPrintCheck()->SetMaxNumberCameraBotImagesHoseNotFound(ui.doubleSpinBoxCameraBotMaxImageHoseNotFound->value());
		GetMainAppPrintCheck()->SetMaxNumberCameraTopImagesHoseNotFound(ui.doubleSpinBoxCameraTopMaxImageHoseNotFound->value());
		GetMainAppPrintCheck()->SetWaitTimeAfterTubEndCameraBotInms(ui.doubleSpinBoxCameraBotWaitTimeAfterTubeEnd->value());
		GetMainAppPrintCheck()->SetWaitTimeAfterTubEndCameraTopInms(ui.doubleSpinBoxCameraTopWaitTimeAfterTubeEnd->value());
	}
}


void WidgetEditGeneralSettings::SlotApplyPixelSizeTop()
{
	if (GetMainAppPrintCheck())
	{
		GetMainAppPrintCheck()->SetPixelSizeCameraTopInMMPerPixel(ui.doubleSpinBoxMeasuredPixelSizeTop->value());
		ui.doubleSpinBoxUsedPixelSizeTop->setValue(GetMainAppPrintCheck()->GetPixelSizeCameraTopInMMPerPixel());
	}
}


void WidgetEditGeneralSettings::SlotApplyPixelSizeBot()
{
	if (GetMainAppPrintCheck())
	{
		GetMainAppPrintCheck()->SetPixelSizeCameraBotInMMPerPixel(ui.doubleSpinBoxMeasuredPixelSizeBot->value());
		ui.doubleSpinBoxUsedPixelSizeBot->setValue(GetMainAppPrintCheck()->GetPixelSizeCameraBotInMMPerPixel());
	}
}

