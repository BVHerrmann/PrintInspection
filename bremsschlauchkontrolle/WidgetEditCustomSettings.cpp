#include "WidgetEditCustomSettings.h"
#include "MainAppPrintCheck.h"
#include "ImageData.h"
#include "qpushbutton.h"


WidgetEditCustomSettings::WidgetEditCustomSettings(MainAppPrintCheck *pParent) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pParent;
	connect(ui.pushButtonApplySettings, &QPushButton::clicked, this, &WidgetEditCustomSettings::SlotApplySettings);
	

	ui.comboBoxCameraTopSimulation->insertItem(0, QString("Off"));
	ui.comboBoxCameraTopSimulation->insertItem(1, QString("On"));
	ui.comboBoxCameraTopSimulation->setCurrentIndex(0);

	ui.comboBoxCameraBotSimulation->insertItem(0, QString("Off"));
	ui.comboBoxCameraBotSimulation->insertItem(1, QString("On"));
	ui.comboBoxCameraBotSimulation->setCurrentIndex(0);
}


WidgetEditCustomSettings::~WidgetEditCustomSettings()
{
}


void WidgetEditCustomSettings::SlotApplySettings()
{
	ImageData  *pImageData = NULL;
	
	if (ui.radioButtonSaveNoImages->isChecked())
	{
		GetMainAppPrintCheck()->SetSaveErrorImagePoolCondition(SAVE_FORMAT_IMAGE_NO_IMAGES);
	}
	else
	{
		if (ui.radioButtonSaveAllImages->isChecked())
		{
			GetMainAppPrintCheck()->SetSaveErrorImagePoolCondition(SAVE_FORMAT_IMAGE_ALL_IMAGES);
		}
		else
		{
			if (ui.radioButtonSaveOnlyBadImages->isChecked())
				GetMainAppPrintCheck()->SetSaveErrorImagePoolCondition(SAVE_FORMAT_IMAGE_ONLY_BAD_IMAGES);
			else
			{
				if (ui.radioButtonSaveOnlyGoodImages->isChecked())
					GetMainAppPrintCheck()->SetSaveErrorImagePoolCondition(SAVE_FORMAT_IMAGE_ONLY_GOOD_IMAGES);
			}
		}
	}
	GetMainAppPrintCheck()->SetNumberFormatsInReferenceImageView(ui.doubleSpinBoxNumberSubFormatsRefImage->value());
	GetMainAppPrintCheck()->SetToggelTimeLiveAndMeasureViewInms(ui.doubleSpinBoxToggleTimeErrorImage->value());
	GetMainAppPrintCheck()->SetNumberEvaluationPeriodFormatNotFound(ui.doubleSpinBoxNumberFormatNotFound->value());
	GetMainAppPrintCheck()->SetSpeedSimulationInMPerMin(ui.doubleSpinBoxSpeedSimulation->value());
	
    GetMainAppPrintCheck()->SetMaxErrorsBehindEachOther(ui.doubleSpinBoxMaxErrorsBehindEachOther->value());
	GetMainAppPrintCheck()->SetIntervallBetweenTwoErrors(ui.doubleSpinBoxIntervalBetweenTowErrorsInms->value());
	GetMainAppPrintCheck()->SetMaxNumberOfImagesInDir(ui.doubleSpinBoxMaxNumberImagesInDir->value());// GetMainAppPrintCheck()->GetMaxNumberOfImagesInDir());

	pImageData = GetMainAppPrintCheck()->GetImageData(CAMERA_BOT_INDEX);
	if (pImageData)
	{
		if (ui.comboBoxCameraBotSimulation->currentIndex() == 1)
		{
			pImageData->StartCameraSimulation(true);
			GetMainAppPrintCheck()->SetCameraBotSimulationOn(true);
			GetMainAppPrintCheck()->ShowVideoPlayerButtons(true, CAMERA_BOT_INDEX);
		}
		else
		{
			pImageData->StartCameraSimulation(false);
			GetMainAppPrintCheck()->SetCameraBotSimulationOn(false);
			GetMainAppPrintCheck()->ShowVideoPlayerButtons(false, CAMERA_BOT_INDEX);
		}
	}

	pImageData = GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX);
	if (pImageData)
	{
		if (ui.comboBoxCameraTopSimulation->currentIndex() == 1)
		{
			pImageData->StartCameraSimulation(true);
			GetMainAppPrintCheck()->SetCameraTopSimulationOn(true);//save value
			GetMainAppPrintCheck()->ShowVideoPlayerButtons(true, CAMERA_TOP_INDEX);
		}
		else
		{
			pImageData->StartCameraSimulation(false);
			GetMainAppPrintCheck()->SetCameraTopSimulationOn(false);//save value
			GetMainAppPrintCheck()->ShowVideoPlayerButtons(false, CAMERA_TOP_INDEX);
		}
	}
}


void WidgetEditCustomSettings::showEvent(QShowEvent *event)
{
	QString OnText  = tr("An");//hier gibt es ein Problem mit der ts Datei
	QString OffText = tr("Aus");

	if (GetMainAppPrintCheck()->IsSimulationCameraTopOn())
		ui.comboBoxCameraTopSimulation->setCurrentIndex(1);
	else
		ui.comboBoxCameraTopSimulation->setCurrentIndex(0);

	if (GetMainAppPrintCheck()->IsSimulationCameraBotOn())
		ui.comboBoxCameraBotSimulation->setCurrentIndex(1);
	else
		ui.comboBoxCameraBotSimulation->setCurrentIndex(0);

	switch (GetMainAppPrintCheck()->GetSaveErrorImagePoolCondition())
	{
	case SAVE_FORMAT_IMAGE_NO_IMAGES:
		ui.radioButtonSaveNoImages->setChecked(true);
		break;
	case SAVE_FORMAT_IMAGE_ALL_IMAGES:
		ui.radioButtonSaveAllImages->setChecked(true);
		break;
	case SAVE_FORMAT_IMAGE_ONLY_BAD_IMAGES:
		ui.radioButtonSaveOnlyBadImages->setChecked(true);
		break;
	case SAVE_FORMAT_IMAGE_ONLY_GOOD_IMAGES:
		ui.radioButtonSaveOnlyGoodImages->setChecked(true);
		break;
	default:
		ui.radioButtonSaveNoImages->setChecked(true);
		break;
	}

	ui.doubleSpinBoxNumberSubFormatsRefImage->setValue(GetMainAppPrintCheck()->GetNumberFormatsInReferenceImageView());
	ui.doubleSpinBoxToggleTimeErrorImage->setValue(GetMainAppPrintCheck()->GetToggelTimeLiveAndMeasureViewInms());
	ui.doubleSpinBoxNumberFormatNotFound->setValue(GetMainAppPrintCheck()->GetNumberEvaluationPeriodFormatNotFound());
	ui.doubleSpinBoxSpeedSimulation->setValue(GetMainAppPrintCheck()->GetSpeedSimulationInMPerMin());
	ui.doubleSpinBoxMaxErrorsBehindEachOther->setValue(GetMainAppPrintCheck()->GetMaxErrorsBehindEachOther());
	ui.doubleSpinBoxIntervalBetweenTowErrorsInms->setValue(GetMainAppPrintCheck()->GetIntervallBetweenTwoErrors());
	ui.doubleSpinBoxMaxNumberImagesInDir->setValue(GetMainAppPrintCheck()->GetMaxNumberOfImagesInDir());

	ui.comboBoxCameraTopSimulation->setItemText(0, OffText);
	ui.comboBoxCameraTopSimulation->setItemText(1, OnText);

	ui.comboBoxCameraBotSimulation->setItemText(0, OffText);
	ui.comboBoxCameraBotSimulation->setItemText(1, OnText);
}



