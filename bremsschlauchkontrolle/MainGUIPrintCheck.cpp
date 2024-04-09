#include "MainGUIPrintCheck.h"
#include "MainAppPrintCheck.h"
#include "ImageData.h"
#include "GlobalConst.h"
#include "WidgetLiveImageData.h"
#include "ProductDialog.h"


//Hauptanwendungseite
MainGUIPrintCheck::MainGUIPrintCheck(MainAppPrintCheck *parent) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
, m_NewHoseDetected(false)
, m_MeasureErrorCounter(0)
, m_ElapsedTimeErrorCounter(0)
, m_WidgetLiveImageDataCameraTop(NULL)
, m_WidgetLiveImageDataCameraBot(NULL)
{
  ui.setupUi(this);
  m_MainAppPrintCheck = parent;
  

  m_WidgetLiveImageDataCameraTop = new WidgetLiveImageData(parent, CAMERA_TOP_INDEX);
  m_WidgetLiveImageDataCameraBot = new WidgetLiveImageData(parent, CAMERA_BOT_INDEX);

  connect(this, &MainGUIPrintCheck::SignalStartMeasuring, this, &MainGUIPrintCheck::SlotStartMeasuring);
  connect(this, &MainGUIPrintCheck::SignalStopMeasuring,  this, &MainGUIPrintCheck::SlotStopMeasuring);

  connect(m_MainAppPrintCheck, &MainAppPrintCheck::SignalShowInspectionErrorTextCamTop, m_WidgetLiveImageDataCameraTop, &WidgetLiveImageData::SlotShowInspectionErrorText);
  connect(m_MainAppPrintCheck, &MainAppPrintCheck::SignalShowInspectionErrorTextCamBot, m_WidgetLiveImageDataCameraBot, &WidgetLiveImageData::SlotShowInspectionErrorText);

  connect(ui.pushButtonResetHoseCounter,       &QPushButton::clicked, this, &MainGUIPrintCheck::SlotResetHoseCounter);
  connect(ui.pushButtonResetLongTimeErrorText, &QPushButton::clicked, this, &MainGUIPrintCheck::SlotClearInfoMeasureError);
  
  SlotStopMeasuring();
  if (ui.frameLiveImageDataTop->layout())
	  ui.frameLiveImageDataTop->layout()->addWidget((QWidget*)(m_WidgetLiveImageDataCameraTop));
  if (ui.frameLiveImageDataBot->layout())
	  ui.frameLiveImageDataBot->layout()->addWidget((QWidget*)(m_WidgetLiveImageDataCameraBot));

  ui.frameDashBoardHoseEnd->hide();
  ui.frameDashBoardHoseMiddle->hide();
  ui.frameDashBoardHoseBeginning->hide();
  ShowInfoMeasureError(false);
 }


void MainGUIPrintCheck::ShowCurrentDateTime(QString &DateTime)
{
	ui.labelCurretDateTime->setText(DateTime);
}


void MainGUIPrintCheck::ShowInfoMeasureError(bool Reset)
{
	if (Reset)
	{
		QFont Font("Times", 45, QFont::Bold);
		ui.textEditErrorMessage->setTextBackgroundColor(Qt::yellow);
		ui.textEditErrorMessage->setCurrentFont(Font);
		ui.textEditErrorMessage->setText(tr("Error!"));
		ui.textEditErrorMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		ui.textEditErrorMessage->setStyleSheet("background-color:yellow;");
		ui.pushButtonResetLongTimeErrorText->show();
		ui.frameDashBoardResetLongTimeError->show();
	}
	else
	{
		ui.textEditErrorMessage->clear();
		ui.textEditErrorMessage->setStyleSheet("background-color:rgb(218, 220, 224);");
		ui.pushButtonResetLongTimeErrorText->hide();
		ui.frameDashBoardResetLongTimeError->hide();
	}
}


void MainGUIPrintCheck::ShowVideoPlayerButtons(bool show,int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
		{
			GetWidgetLiveImageDataCameraTop()->ShowVideoPlayerButtons(show);
		}
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
		{
			GetWidgetLiveImageDataCameraBot()->ShowVideoPlayerButtons(show);
		}
	}
}


void MainGUIPrintCheck::SetSliderValues(int MaxValue, int Number, int CameraIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
		{
			GetWidgetLiveImageDataCameraTop()->SetSliderValues(MaxValue, Number);
		}
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
		{
			GetWidgetLiveImageDataCameraBot()->SetSliderValues(MaxValue, Number);
		}
	}
}


void MainGUIPrintCheck::SlotResetHoseCounter()
{
	ui.doubleSpinBoxNumberHose->setValue(0.0);
	ui.doubleSpinBoxNumberErrorHose->setValue(0.0);
}


void MainGUIPrintCheck::IncrementNumberHose()
{
	m_NewHoseDetected = true;
	ui.doubleSpinBoxNumberHose->setValue(ui.doubleSpinBoxNumberHose->value() + 1.0);
}


int MainGUIPrintCheck::GetNumberHose()
{
    return ui.doubleSpinBoxNumberHose->value();
}

//wird aufgerufen, wenn ein Fehler auf dem Schlauch erkannt
void MainGUIPrintCheck::IncrementNumberErrorHose()
{
	qint64 currentTime, ErrorInterval;
	if (m_NewHoseDetected)
	{
		ui.doubleSpinBoxNumberErrorHose->setValue(ui.doubleSpinBoxNumberErrorHose->value() + 1.0);
		m_NewHoseDetected = false;
	}

	if (!m_TimerErrorCounter.isValid())
		m_TimerErrorCounter.start();
	currentTime   = m_TimerErrorCounter.elapsed();
	ErrorInterval = currentTime - m_ElapsedTimeErrorCounter;

	m_ElapsedTimeErrorCounter = currentTime;
	if (ErrorInterval < GetMainAppPrintCheck()->GetIntervallBetweenTwoErrors())
		m_MeasureErrorCounter++;
	else
	{
		m_MeasureErrorCounter = 1;
		m_TimerErrorCounter.start();
		m_ElapsedTimeErrorCounter = 0;
	}

	if (m_MeasureErrorCounter > GetMainAppPrintCheck()->GetMaxErrorsBehindEachOther())
	{
		ShowInfoMeasureError(true);
		m_MeasureErrorCounter = 0;
		m_TimerErrorCounter.start();
		m_ElapsedTimeErrorCounter = 0;
	}
}


void MainGUIPrintCheck::SlotClearInfoMeasureError()
{
	ShowInfoMeasureError(false);
}


void MainGUIPrintCheck::SetSignalStartMeasuring()
{
	emit SignalStartMeasuring();
}


void MainGUIPrintCheck::SetSignalStopMeasuring()
{
	emit SignalStopMeasuring();
}


void MainGUIPrintCheck::SetCameraStatus(int CameraIndex, QString Text,bool Simulation)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
		GetWidgetLiveImageDataCameraTop()->SetStatusLiveCamera(Text, Simulation);
	else
		GetWidgetLiveImageDataCameraBot()->SetStatusLiveCamera(Text, Simulation);
}


void MainGUIPrintCheck::SlotStartMeasuring()
{
	HideWidgetMeasureIsRunning(true);
	if (GetMainAppPrintCheck())
	{
		for (int i = 0; i < NUMBER_CAMERAS; i++)
		{
			if (GetMainAppPrintCheck()->GetImageData(i))
				GetMainAppPrintCheck()->GetImageData(i)->ResumeMeasuring(true);
		}
		GetMainAppPrintCheck()->SetEnableShowProductWindow(false);
		GetMainAppPrintCheck()->InspectionIsActiveToPLC(true);
		if (GetWidgetLiveImageDataCameraTop())
		    GetWidgetLiveImageDataCameraTop()->ShowButtonSaveVideo(false);
	    if (GetWidgetLiveImageDataCameraBot())
		    GetWidgetLiveImageDataCameraBot()->ShowButtonSaveVideo(false);
		GetMainAppPrintCheck()->SetCurrentMaschineState(PluginInterface::MachineState::Production);
    }
}


ProductDialog *MainGUIPrintCheck::GetProductDialog()
{
	if (GetMainAppPrintCheck())
		return GetMainAppPrintCheck()->GetProductDialog();
	else
		return NULL;
}


void MainGUIPrintCheck::SlotStopMeasuring()
{
	HideWidgetMeasureIsRunning(false);
	if (GetMainAppPrintCheck())
	{
		for (int i = 0; i < NUMBER_CAMERAS; i++)
		{
			if (GetMainAppPrintCheck()->GetImageData(i))
			{
				GetMainAppPrintCheck()->GetImageData(i)->ResumeMeasuring(false);
			}
		}
		GetMainAppPrintCheck()->SetEnableShowProductWindow(true);
		if (GetWidgetLiveImageDataCameraTop())
	    	GetWidgetLiveImageDataCameraTop()->ShowButtonSaveVideo(true);
	    if (GetWidgetLiveImageDataCameraBot())
		    GetWidgetLiveImageDataCameraBot()->ShowButtonSaveVideo(true);
	    GetMainAppPrintCheck()->SetCurrentMaschineState(PluginInterface::MachineState::Setup);
    }
}

 
void MainGUIPrintCheck::HideWidgetMeasureIsRunning(bool set)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->HideWidgetMeasureIsRunning(set);
}


void MainGUIPrintCheck::AddLiveImageWidget(int CameraIndex, QWidget *w)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->AddLiveImageWidget(w);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->AddLiveImageWidget(w);
	}
}


void MainGUIPrintCheck::AddErrorImageWidget(int CameraIndex, QWidget *w)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->AddErrorImageWidget(w);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->AddErrorImageWidget(w);
	}
}


void MainGUIPrintCheck::AddCameraImageWidget(int CameraIndex, QWidget *w)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->AddCameraImageWidget(w);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->AddCameraImageWidget(w);
	}
}


void MainGUIPrintCheck::SetHoseStart(bool set)
{
	ui.radioButtonHoseBeginning->setChecked(set);
	QTimer::singleShot(500, this, SLOT(SlotResetHoseStart()));
}


void MainGUIPrintCheck::SetHoseMiddle(bool set)
{
	ui.radioButtonHoseMiddle->setChecked(set);
}


void MainGUIPrintCheck::SetHoseEnd(bool set)
{
	ui.radioButtonHoseEnd->setChecked(set);
	QTimer::singleShot(500, this, SLOT(SlotResetHoseEnd()));
}


void MainGUIPrintCheck::SlotResetHoseEnd()
{
	ui.radioButtonHoseEnd->setChecked(false);
}


void MainGUIPrintCheck::SlotResetHoseStart()
{
	ui.radioButtonHoseBeginning->setChecked(false);
}


void MainGUIPrintCheck::ShowImageCounter(int CameraIndex, double set)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetFormatCount(static_cast<int>(set));
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetFormatCount(static_cast<int>(set));
	}
}


void MainGUIPrintCheck::SetTabWidget(int CameraIndex, int TabIndex)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetTabWidget(TabIndex);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetTabWidget(TabIndex);
	}
}


bool MainGUIPrintCheck::IsCheckedShowOnlyErrorImage(int CameraIndex)
{
	bool rv = false;

	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			rv = GetWidgetLiveImageDataCameraTop()->IsCheckedShowOnlyErrorImage();// SetTabWidget(TabIndex);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			rv = GetWidgetLiveImageDataCameraBot()->IsCheckedShowOnlyErrorImage();//SetTabWidget(TabIndex);
	}
	return rv;
}


void MainGUIPrintCheck::SetInspectionTime(int CameraIndex, double set)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetInspectionTime(set);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetInspectionTime(set);
	}
}


void MainGUIPrintCheck::SetMeanDefectScore(int CameraIndex,double set)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetMeanDefectScore(set);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetMeanDefectScore(set);
	}
}


void MainGUIPrintCheck::SetMaxDefectScore(int CameraIndex, double set)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetMaxDefectScore(set);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetMaxDefectScore(set);
	}
}


void MainGUIPrintCheck::SetCameraFramesPerSecond(int CameraIndex, double set)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetCameraFramesPerSecond(set);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetCameraFramesPerSecond(set);
	}
}


void MainGUIPrintCheck::SetMaxCenterOffset(int CameraIndex, double set)
{
	if (CameraIndex == CAMERA_TOP_INDEX)
	{
		if (GetWidgetLiveImageDataCameraTop())
			GetWidgetLiveImageDataCameraTop()->SetMaxCenterOffset(set);
	}
	else
	{
		if (GetWidgetLiveImageDataCameraBot())
			GetWidgetLiveImageDataCameraBot()->SetMaxCenterOffset(set);
	}
}










