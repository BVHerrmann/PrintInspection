#include "WidgetLiveImageData.h"
#include "MainAppPrintCheck.h"
#include "GlobalConst.h"
#include "ImageData.h"
#include "ProductData.h"
#include "SaveVideoDialog.h"
#include "CameraSimulation.h"
#include "ErrorImageView.h"



WidgetLiveImageData::WidgetLiveImageData(MainAppPrintCheck *pParent, int CameraIndex) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
, m_WindowSetup(false)
, m_SaveVideoDialog(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pParent;
	m_CameraIndex       = CameraIndex;

	connect(ui.checkBoxShowErrorImage,             &QCheckBox::stateChanged, this, &WidgetLiveImageData::SlotStateShowErrorImage);
	connect(ui.pushButtonShowDetailResults,        &QPushButton::clicked,    this, &WidgetLiveImageData::SlotShowDetailResults);
	connect(ui.pushButtonSaveVideo,                &QPushButton::clicked,    this, &WidgetLiveImageData::SlotSaveVideo);
	connect(ui.pushButtonStartStop,                &QPushButton::toggled,    this, &WidgetLiveImageData::SlotStartStopVideo);
	connect(ui.pushButtonForward,                  &QPushButton::clicked,    this, &WidgetLiveImageData::SlotStepForward);
	connect(ui.horizontalSliderCurrentVideoFrame,  &QSlider::sliderReleased, this, &WidgetLiveImageData::SlotSliderReleased);
	connect(ui.horizontalSliderCurrentVideoFrame,  &QSlider::sliderMoved,    this, &WidgetLiveImageData::SlotSliderMoved);
	connect(ui.pushButtonClearErrorImage,          &QPushButton::clicked,    this, &WidgetLiveImageData::SlotClearErrorImage);

	ShowButtonSaveVideo(false);
	ui.pushButtonSaveVideo->setProperty(kRequiredAccessLevel, kAccessLevelAdmin);
	ui.groupBoxVideoPlayer->hide();
	ui.pushButtonClearErrorImage->hide();//nur sichtbar wenn Fehlerbild angezeigt
	ui.pushButtonForward->hide();
}


WidgetLiveImageData::~WidgetLiveImageData()
{
}


void WidgetLiveImageData::SetSliderValues(int MaxValue,int Number)
{
	ui.horizontalSliderCurrentVideoFrame->setMaximum(MaxValue);
	ui.horizontalSliderCurrentVideoFrame->setValue(Number);
	ui.lcdNumberCurrentVideoFrameNumber->display(Number);
}


void WidgetLiveImageData::SlotSliderReleased()
{
	int Value = ui.horizontalSliderCurrentVideoFrame->sliderPosition();
	if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation())
		{
			GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation()->SetImageIndexSubImageFromFile(Value);
			GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->ClearSharedMemory();
			ui.lcdNumberCurrentVideoFrameNumber->display(Value);
		}
	}
}


void  WidgetLiveImageData::SlotSliderMoved(int value)
{
	ui.lcdNumberCurrentVideoFrameNumber->display(value);
}


void WidgetLiveImageData::ShowVideoPlayerButtons(bool show)
{
	if(show)
		ui.groupBoxVideoPlayer->show();
	else
		ui.groupBoxVideoPlayer->hide();
}


void WidgetLiveImageData::SlotStartStopVideo(bool toggle)
{
	if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation())
		{
			if (toggle)
			{
				ui.pushButtonStartStop->setText(tr("Start Simulation"));
				GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation()->SetVideoState(STOP_VIDEO);
				ui.pushButtonForward->show();
				ui.horizontalSliderCurrentVideoFrame->setEnabled(true);
			}
			else
			{
				ui.pushButtonStartStop->setText(tr("Stop Simulation"));
				GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation()->SetVideoState(PLAY_VIDEO);
				ui.pushButtonForward->hide();
				ui.horizontalSliderCurrentVideoFrame->setEnabled(false);
			}
		}
	}
}


void WidgetLiveImageData::SlotStepForward()
{
	if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation())
		{
			GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetCameraSimulation()->SetStepOneImage();
    	}
	}
}


void WidgetLiveImageData::SetFormatCount(int count)
{
	ui.doubleSpinBoxFormatCounter->setValue(count);
}


void WidgetLiveImageData::SlotShowInspectionErrorText(const QString &Text,bool Error)
{
	if (Error)
	{
		ui.labelDashBoardErrorText->setStyleSheet("font-weight: bold; color: red;  background-color: white");
		QTimer::singleShot(3000, this, SLOT(SlotResetText()));
	}
	else
	{
		ui.labelDashBoardErrorText->setStyleSheet("font-weight: bold; color: green;  background-color: white");
	}
	ui.labelDashBoardErrorText->setText(Text);
}


void WidgetLiveImageData::SlotResetText()
{
	ui.labelDashBoardErrorText->setText("     ");
}


void WidgetLiveImageData::ShowButtonSaveVideo(bool show)
{
		ui.pushButtonSaveVideo->setEnabled(show);
}


void WidgetLiveImageData::showEvent(QShowEvent *event)
{
	ProductData *pProductData = GetMainAppPrintCheck()->GetCurrentProductData();
	if (pProductData)
	{
		SetTabWidget(TAB_INDEX_ERROR_IMAGE);//einmal Fehlerbildfenster zur Anzeige bringen, damit die Fenstergröße angepasst wird
		SetTabWidget(TAB_INDEX_LIVE_IMAGE);
		disconnect(ui.checkBoxShowErrorImage, &QCheckBox::stateChanged, this, &WidgetLiveImageData::SlotStateShowErrorImage);
		ui.checkBoxShowErrorImage->setChecked(false);
		connect(ui.checkBoxShowErrorImage, &QCheckBox::stateChanged, this, &WidgetLiveImageData::SlotStateShowErrorImage);
		if (m_CameraIndex == CAMERA_TOP_INDEX)
		{
			if (!GetMainAppPrintCheck()->IsSimulationCameraTopOn())
				SetStatusLiveCamera(tr("Camera Live Image"), false);
		}
		else
		{
			if (!GetMainAppPrintCheck()->IsSimulationCameraBotOn())
				SetStatusLiveCamera(tr("Camera Live Image"), false);
		}
	}
}


void WidgetLiveImageData::SlotStateShowErrorImage(int State)
{
	if (State == Qt::Checked)
	{
		SetTabWidget(TAB_INDEX_ERROR_IMAGE);
		ui.pushButtonClearErrorImage->show();
	}
	else
	{
		SetTabWidget(TAB_INDEX_LIVE_IMAGE);
		ui.pushButtonClearErrorImage->hide();
	}
}


void WidgetLiveImageData::SlotSaveVideo()
{
	if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		bool rv=GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetEnableWriteFullHose();
		if (rv)
		{
			ui.pushButtonSaveVideo->setText(tr("Save Video"));
			ui.pushButtonSaveVideo->setStyleSheet("font-weight: bold; color: white;  background-color:rgb(64, 77, 83)");
		}
		else
		{
			ui.pushButtonSaveVideo->setText(tr("Save Video Is On"));
			ui.pushButtonSaveVideo->setStyleSheet("font-weight: bold; color: red;  background-color: white");
		}
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->SetEnableWriteFullHose(!rv);
	}
}


void WidgetLiveImageData::SetTabWidget(int TabIndex)
{
	ui.stackedWidget->setCurrentIndex(TabIndex);
}


void WidgetLiveImageData::SlotShowDetailResults()
{
	if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->ShowDetailResults();
	}
}


void WidgetLiveImageData::SlotClearErrorImage()
{
	if (GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetErrorImageView()->ClearImage();
	}
}


void WidgetLiveImageData::SetStatusLiveCamera(QString &Text,bool Simulation)
{
	if (Simulation)
	{
		ui.labelDashBoardCameraStatus->setStyleSheet("font-weight: bold; color: red;  background-color: white");
	}
	else
	{
		ProductData *pProductData = GetMainAppPrintCheck()->GetCurrentProductData();
		if (pProductData)
		{
			if(m_CameraIndex == CAMERA_TOP_INDEX)
			   Text = Text + QString("(%1)").arg(pProductData->m_CameraTopType);
			else
			   Text = Text + QString("(%1)").arg(pProductData->m_CameraBotType);
		}
		ui.labelDashBoardCameraStatus->setStyleSheet("font-weight: bold; color: black;  background-color: white");
	}
	ui.labelDashBoardCameraStatus->setText(Text);
}


void WidgetLiveImageData::AddLiveImageWidget(QWidget *w)
{
	ui.LiveImageFrame->layout()->addWidget(w);
}


void WidgetLiveImageData::AddErrorImageWidget(QWidget *w)
{
	ui.ErrorImageFrame->layout()->addWidget(w);
}


void WidgetLiveImageData::AddCameraImageWidget(QWidget *w)
{
	ui.CameraImageFrame->layout()->addWidget(w);
}


bool WidgetLiveImageData::IsCheckedShowOnlyErrorImage()
{
	if (ui.checkBoxShowErrorImage->checkState() == Qt::Checked)
	    return true;
	else
	    return false;
}


void WidgetLiveImageData::SetInspectionTime(double set)
{
	ui.doubleSpinBoxInspectionTime->setValue(set);
}


void WidgetLiveImageData::SetMeanDefectScore(double set)
{
	// ui.doubleSpinBoxMeanDefectScore->setValue(set);
}


void WidgetLiveImageData::SetMaxDefectScore(double set)
{
   // ui.doubleSpinBoxMaxDefectScore->setValue(set);
}


void WidgetLiveImageData::SetCameraFramesPerSecond(double set)
{
	if (set > 0.0)
	{
		double Value = 1000.0 / set;//Camera intervall in ms
		ui.doubleSpinBoxHoseDiameter->setValue(Value);
	}
}


void WidgetLiveImageData::SetMaxCenterOffset(double set)
{
	//ui.doubleSpinBoxCenterOffset->setValue(set);
}
