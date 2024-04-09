#ifndef MAINLOGIC_H
#define MAINLOGIC_H

#include "logic.h"
#include <interfaces.h>
#include <QtCore>
#include "ImageData.h"
#include "ImageLineInformation.h"


class MainAppPrintCheck;
class MainLogic : public Logic
{
	Q_OBJECT
public:
	explicit MainLogic(MainAppPrintCheck *pMainAppPrintCheck);

	void setCurrentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState);
    
    void AppendLineInformation(QList<ImageLineInformation> &lines, int CameraIndex);
    
	//void SetTubeStartFound(bool set)               { m_EnableTubeStartFound = set; }
	//void SetTubeEndFound(bool set)                 { m_EnableTubeEndFound = set; }
	//void SetTubeMiddleFound(bool set)              { m_TubeMiddleFound = set; }
    void SetAckInspectionActive(bool set)           { m_AckInspectionActive = set;}
	void SetAckJobID(int set)                      { m_AckJobID = set; }

	/*void SetCamTopFontError(bool set)              { m_CamTopFontError = set;}
	void SetCamTopPositionError(bool set)          { m_CamTopPositionError = set;	}
	void SetCamTopLineError(bool set)              { m_CamTopLineError = set; }
	void SetCamTopFontHeightError(bool set)        { m_CamTopFontHeightError = set; }
	void SetCamTopFontWidthError(bool set)         { m_CamTopFontWidthError = set; }
	void SetCamTopFormatNotFoundError(bool set)    { m_CamTopFormatNotFoundError = set; }

	void SetCamBotFontError(bool set)              { m_CamBotFontError = set; }
	void SetCamBotPositionError(bool set)          { m_CamBotPositionError = set; }
	void SetCamBotLineError(bool set)              { m_CamBotLineError = set; }
	void SetCamBotFontHeightError(bool set)        { m_CamBotFontHeightError = set; }
	void SetCamBotFontWidthError(bool set)         { m_CamBotFontWidthError = set; }
	void SetCamBotFormatNotFoundError(bool set)    { m_CamBotFormatNotFoundError = set; }
	*/

	
	void ClearResults();
	void ClearResultsTop();
	void ClearResultsBot();
	bool SetCurrentMeasuringResults();

	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }

	void IsDoWorkRunning();

	void SetGetIO();
	unsigned __int64 GetCurrentTimeStamp();

	void SetCurrentLineSpeed(double set) { m_CurrentLineSpeed = set; }
    
signals:
    // CommunicationInterface
    void valueChanged(const QString &name, const QVariant &value);

    void SignalLoadJob(int job_id);
	void SignalInspectionActive(bool Active);
	void SignalResetFault();
	void SignalCurrentSpeed(double speed);
	void SignalTubeEndReached();
    
public slots:

private:
	MainAppPrintCheck *m_MainAppPrintCheck;
    int  m_JobID;
    int  m_AckJobID;
    bool m_ResetFault;
	bool m_AckInspectionActive;
	bool m_InspectionActive;
	bool m_InspectionSlot;
	bool m_IsDoWorkRunning;
	bool m_TubeEndReached;
	bool m_Toggle;
	double m_CurrentLineSpeed;
	unsigned long m_TestCounter;
	

    uint m_AccessLevel;
    PluginInterface::MachineState m_MachineState;
    PluginInterface::DiagState    m_DiagState;
    
    std::mutex m_MutexLinesTop;
    std::mutex m_MutexLinesBottom;
    QList<ImageLineInformation> m_LinesTop;
    QList<ImageLineInformation> m_LinesBottom;

	QWaitCondition            m_WaitConditionDoWorkIsRunning;
	QMutex                    m_MutexDoWorkIsRunning;
	
    
    INPUT(int, coordination)
    INPUT(bool, admin)
    INPUT(bool, service)
    INPUT(bool, reset_fault)
    INPUT(bool, inspection_active)
    INPUT(bool, inspection_disable)
    INPUT(bool, tube_start)
    INPUT(bool, tube_stop)
    INPUT(bool, inspection_slot)
    INPUT(int, job_id)
    INPUT(double, line_speed)

    OUTPUT(int, ack_coordination, 0)
    
    OUTPUT(bool, software_started, false)
    OUTPUT(bool, has_warning, false)
    OUTPUT(bool, has_fault, false)
    OUTPUT(bool, inspection_is_active, false)
    OUTPUT(bool, tube_end_detected, false)
    OUTPUT(bool, tube_start_detected, false)
    OUTPUT(bool, tube_detected, false)

    OUTPUT(int, ack_job_id, false)
    
    OUTPUT(bool, cam_top_font, false)
    OUTPUT(bool, cam_top_position, false)
    OUTPUT(bool, cam_top_line, false)
    OUTPUT(bool, cam_top_font_height, false)
    OUTPUT(bool, cam_top_font_width, false)
    OUTPUT(bool, cam_top_no_format, false)
    
    OUTPUT(bool, cam_bottom_font, false)
    OUTPUT(bool, cam_bottom_position, false)
    OUTPUT(bool, cam_bottom_line, false)
    OUTPUT(bool, cam_bottom_font_height, false)
    OUTPUT(bool, cam_bottom_font_width, false)
    OUTPUT(bool, cam_bottom_no_format, false)
    
    void doWork();
    void watchdog();
};




#endif // MAINLOGIC_H
