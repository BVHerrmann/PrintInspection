#pragma once
#include <qwidget.h>
#include "ui_WidgetLiveImageData.h"


class MainAppPrintCheck;
class SaveVideoDialog;
class WidgetLiveImageData :	public QWidget
{
	Q_OBJECT
public:
	WidgetLiveImageData(MainAppPrintCheck *pParent, int CameraInde);
	~WidgetLiveImageData();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	SaveVideoDialog   *GetSaveVideoDialog()   { return m_SaveVideoDialog; }
	void showEvent(QShowEvent *event);
	int GetCameraIndex() { return m_CameraIndex; }
	void SetTabWidget(int TabIndex);
	void AddLiveImageWidget(QWidget *w);
	void AddErrorImageWidget(QWidget *w);
	void AddCameraImageWidget(QWidget *w);
	bool IsCheckedShowOnlyErrorImage();
	void SetInspectionTime(double set);
	void SetMeanDefectScore(double set);
	void SetMaxDefectScore(double set);
	void SetCameraFramesPerSecond(double set);
	void SetMaxCenterOffset(double set);
	void ShowButtonSaveVideo(bool show);
	void SetStatusLiveCamera(QString &Text,bool Simulation);
	void SetFormatCount(int count);
	void ShowVideoPlayerButtons(bool show);
	void SetSliderValues(int MaxValue, int Number);

public slots:
	void SlotStateShowErrorImage(int);
	void SlotShowDetailResults();
	void SlotSaveVideo();
	void SlotShowInspectionErrorText(const QString &Text,bool Error);
	void SlotResetText();
	void SlotStartStopVideo(bool);
	void SlotStepForward();
	void SlotSliderReleased();
	void SlotSliderMoved(int value);
	void SlotClearErrorImage();

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
	SaveVideoDialog   *m_SaveVideoDialog;
	int m_CameraIndex;
	bool         m_WindowSetup;
	Ui::WidgetLiveImageData ui;
};

