#ifndef BREMS_SCHLAUCH_KONTROLLE_PLUGIN_H
#define BREMS_SCHLAUCH_KONTROLLE_PLUGIN_H

#include <plugin.h>
#include <interfaces.h>
#include <QObject>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "GlobalConst.h"


class MainAppPrintCheck;
class WidgetEditReferenceData;
class WidgetEditProductData;
class WidgetEditCustomSettings;
class WidgetEditGeneralSettings;
class WidgetEditMeasureParameter;
class MainGUIPrintCheck;
class BremsSchlauchKontrollePlugin : public Plugin, MainWindowInterface, PluginInterface, CommunicationInterface, ImageConsumerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.bertram-bildverarbeitung.BremsschlauchkontrollePlugin")
    Q_INTERFACES(MainWindowInterface)
    Q_INTERFACES(PluginInterface)
    Q_INTERFACES(CommunicationInterface)
    Q_INTERFACES(ImageConsumerInterface)

public:
    explicit BremsSchlauchKontrollePlugin(QObject *parent = 0);
    virtual ~BremsSchlauchKontrollePlugin();

    // PluginWindowInterface
    const QString identifier() const { return "Bremsschlauchkontrolle"; }
	const QString name()const { return "Bremsschlauchkontrolle"; }
    QThread::Priority priority() const { return QThread::IdlePriority; }
	const QString currentProductName() const { return m_CurrentProductName; }
	void SetMessage(const QString &message, QtMsgType MsgType);
	bool canShowProductWindow() const { return m_EnableShowProductWindow; }
	const MachineState machineState() const { return m_CurrentMaschineState; }

	// MainWindowInterface
	const WidgetType widgetType(const int idx) const
	{
		if (idx == TAB_INDEX_MAIN_WIDGET_GENERAL_SETTINGS)
			return Settings;
		else
		     return Application; 
	}
	const QString title(const int idx = 0) const;
	QWidget *mainWidget(const int idx = 0) const;
	int requiredWidgetAccessLevel(const int idx = 0) const;
    int mainWidgetCount() const { return 7; }
	MainAppPrintCheck        *GetMainAppPrintCheck()        { return m_MainAppPrintCheck; }
	void SetCurrentProductName(QString &set) { m_CurrentProductName = set; }
	void SetPreference(const QString & preference, const QVariant & value);
	QVariant GetPreference(const QString & preference);
	void HideWidgetMeasureIsRunning(bool set);
	void SetEnableShowProductWindow(bool set) { m_EnableShowProductWindow = set; }
	
	int  GetCameraWidthInPixel(int index)  { return m_CameraWidthInPixel[index]; }
	int  GetCameraHeightInPixel(int index) { return m_CameraHeightInPixel[index];}
	int  GetBinningHorizontal(int index)   { return m_BinningHorizontal[index]; }
	int  GetBinningVertical(int index)     { return m_BinningVertical[index]; }
	int  GetAcquisitionLineRate(int index) { return m_AcquisitionLineRate[index]; }
	int  GetExposureTime(int index)        { return m_ExposureTime[index]; }

	void SetCameraDeviceWidthInPixel(int set, int index);
	void SetCameraDeviceHeightInPixel(int set, int index);
	void SetBinningHorizontal(int set, int index);
	void SetBinningVertical(int set, int index);
	void SetAcquisitionLineRate(int set, int index);
	void SetExposureTime(int set, int index);

	void SetCurrentMaschineState(MachineState set) {m_CurrentMaschineState = set;}

signals:
    // CommunicationInterface
    void valuesChanged(const QHash<QString, QVariant> &values);
    void valueChanged(const QString &name, const QVariant &value);
    
public slots:
    // PluginInterface
    void initialize();
    void uninitialize();
    void currentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState);
	void showProductWindow();
	void reset();
	void requestMachineState(const PluginInterface::MachineState state);
	

    // PreferencesInterface
    void loadPreferences();

    // CommunicationInterface
    void setValue(const QString &name, const QVariant &value);
    void setValues(const QHash<QString, QVariant> &values);
	
    // ImageConsumerInterface
    void consumeImage(const QString &cameraId, unsigned long frameNumber, int frameStatus, const cv::Mat &image);

private:
  	WidgetEditReferenceData    *m_WidgetEditReferenceData;
	WidgetEditProductData      *m_WidgetEditProductData;
	WidgetEditCustomSettings   *m_WidgetEditCustomSettings;
	WidgetEditGeneralSettings  *m_WidgetEditGeneralSettings;
	WidgetEditMeasureParameter *m_WidgetEditMeasureParameterCameraTop;
	WidgetEditMeasureParameter *m_WidgetEditMeasureParameterCameraBot;
	MainGUIPrintCheck          *m_MainGUIPrintCheck;
	MainAppPrintCheck          *m_MainAppPrintCheck;
	QString                     m_CurrentProductName;
	QString                     m_CameraIDTop, m_CameraIDBot;
	bool                        m_HideWidgetMeasureIsRunning;
	bool                        m_EnableShowProductWindow;
	MachineState                m_CurrentMaschineState;
	//CameraParameter
	int                         m_CameraWidthInPixel[NUMBER_CAMERAS];
	int                         m_CameraHeightInPixel[NUMBER_CAMERAS];
	int                         m_BinningHorizontal[NUMBER_CAMERAS];
	int                         m_BinningVertical[NUMBER_CAMERAS];
	int                         m_AcquisitionLineRate[NUMBER_CAMERAS];
	int                         m_ExposureTime[NUMBER_CAMERAS];
	int                         m_HeartbeatTimeout[NUMBER_CAMERAS];
};

#endif 
