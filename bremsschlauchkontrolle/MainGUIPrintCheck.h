#ifndef BREMS_SCHLAUCH_KONTROLLE_WIDGET_H
#define BREMS_SCHLAUCH_KONTROLLE_WIDGET_H

#include <QWidget>
#include <qpushbutton.h>
#include "ui_MainGUIPrintCheck.h"
#include "qelapsedtimer.h"


class WidgetLiveImageData;
class MainAppPrintCheck;
class ProductDialog;
class MainGUIPrintCheck : public QWidget
{
Q_OBJECT
public:
    explicit MainGUIPrintCheck(MainAppPrintCheck *parent);
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	WidgetLiveImageData *GetWidgetLiveImageDataCameraTop() { return m_WidgetLiveImageDataCameraTop; }
	WidgetLiveImageData *GetWidgetLiveImageDataCameraBot() { return m_WidgetLiveImageDataCameraBot;	}

	
	void AddLiveImageWidget(int CameraIndex, QWidget *w);
	void AddErrorImageWidget(int CameraIndex, QWidget *w);
	void AddCameraImageWidget(int CameraIndex, QWidget *w);
	void SetTabWidget(int CameraIndex,int TabIndex);
	void HideWidgetMeasureIsRunning(bool set);

	void SetHoseStart(bool set);
	void SetHoseMiddle(bool set);
	void SetHoseEnd(bool set);

	void IncrementNumberHose();
	void IncrementNumberErrorHose();
	//void ResetNumberHose();

	void SetInspectionTime(int CameraIndex, double set);
	void SetMeanDefectScore(int CameraIndex, double set);
	void SetMaxDefectScore(int CameraIndex, double set);
	void SetCameraFramesPerSecond(int CameraIndex, double set);
	void SetMaxCenterOffset(int CameraIndex, double set);
	void ShowImageCounter(int CameraIndex, double set);

	//void SetTextFormatImage(int CameraIndex,int TabIndex);

	bool IsCheckedShowOnlyErrorImage(int CameraIndex);
	//void WriteImageFullFormatIsFinished(int CameraIndex);

	ProductDialog *GetProductDialog();
	void SetSignalStartMeasuring();
	void SetSignalStopMeasuring();

	void SetCameraStatus(int CameraIndex, QString Text,bool Simulation);

	void ShowVideoPlayerButtons(bool show, int CameraIndex);
	void SetSliderValues(int MaxValue, int Number, int CameraIndex);

	void ShowInfoMeasureError(bool Reset);
	int  GetNumberHose();

	void ShowCurrentDateTime(QString &DateTime);
	

public slots:
	void SlotStartMeasuring();
	void SlotStopMeasuring();
	void SlotResetHoseEnd();
	void SlotResetHoseStart();
	void SlotResetHoseCounter();
	void SlotClearInfoMeasureError();

signals:
	void SignalStartMeasuring();
	void SignalStopMeasuring();
	
	
	
private:
    MainAppPrintCheck *m_MainAppPrintCheck;
	WidgetLiveImageData *m_WidgetLiveImageDataCameraTop;
	WidgetLiveImageData *m_WidgetLiveImageDataCameraBot;
	Ui::MainGUIPrintCheck ui;
	bool m_NewHoseDetected;
	int m_MeasureErrorCounter;
	qint64 m_ElapsedTimeErrorCounter;
	QElapsedTimer m_TimerErrorCounter;
};

#endif 
