#include "MainLogic.h"

#include <QtCore>

#include "GlobalConst.h"
#include "MainAppPrintCheck.h"


MainLogic::MainLogic(MainAppPrintCheck *pMainAppPrintCheck) : Logic()
, m_AckJobID(-1)
, m_InspectionActive(false)
, m_AckInspectionActive(false)
, m_JobID(-1)
, m_ResetFault(false)
, m_InspectionSlot(false)
, m_AccessLevel(kAccessLevelGuest)
, m_MainAppPrintCheck(NULL)
, m_IsDoWorkRunning(false)
, m_Toggle(false)
, m_CurrentLineSpeed(0.0)
, m_TubeEndReached(false)
, m_TestCounter(0)
{
    QTimer *watchdog = new QTimer(this);

	m_MainAppPrintCheck = pMainAppPrintCheck;
    connect(watchdog, &QTimer::timeout, this, &MainLogic::watchdog);
	if(!m_MainAppPrintCheck->IsPLCSimulationOn())
 	    watchdog->start(100);
}


void MainLogic::doWork()
{
	SetGetIO();
}


void MainLogic::SetGetIO()
{
	bool slot_flank = false;

	m_IsDoWorkRunning = true;
    // return coordination value
	set_ack_coordination(coordination());
    // job handling
    set_ack_job_id(m_AckJobID);
    // error handling
    set_software_started(m_MachineState >= PluginInterface::Setup);
    set_has_warning(m_DiagState == PluginInterface::WarningLow || m_DiagState == PluginInterface::WarningHigh);
    set_has_fault(m_DiagState == PluginInterface::Alarm);

	//Setze Schlauchanfang, Schlauchende, SchlauchMitte
	if (tube_start())//Vorankündigung Schlauchanfang
	{
			set_tube_start_detected(true);//Setze Schlauchanfang Bit
    }
	else
	{
	    	set_tube_start_detected(false);//Rücksetzen Sclauchanfang
	}
	if (tube_stop())//Vorankündigung Schlauchende
	{
			set_tube_end_detected(true);
			m_TubeEndReached = true;
	}
	else
	{
			set_tube_end_detected(false);//Rücksetzen Sclauchende
	}

	if (m_InspectionSlot != inspection_slot())
	{
        slot_flank = true;
		m_InspectionSlot = inspection_slot();
	}
	
	set_inspection_is_active(m_AckInspectionActive);

   	if (inspection_disable())
	{
		set_tube_detected(false);

		set_cam_top_font(false);
		set_cam_top_position(false);
		set_cam_top_line(false);
		set_cam_top_font_height(false);
		set_cam_top_font_width(false);
		set_cam_top_no_format(false);

		set_cam_bottom_font(false);
		set_cam_bottom_position(false);
		set_cam_bottom_line(false);
		set_cam_bottom_font_height(false);
		set_cam_bottom_font_width(false);
		set_cam_bottom_no_format(false);
		//Lösche Ergebnisse wenn Messung unterbrochen
		ClearResults();
    }
	else
	{	
		if (slot_flank)
		{
			SetCurrentMeasuringResults();
		}
	}

    if (reset_fault() && !m_ResetFault)
	{
		m_ResetFault = true;
    }
}


void MainLogic::IsDoWorkRunning()
{ 
	m_MutexDoWorkIsRunning.lock();
	if (m_WaitConditionDoWorkIsRunning.wait(&m_MutexDoWorkIsRunning, 1000))
		m_IsDoWorkRunning = true;
	else
		m_IsDoWorkRunning = false;
	m_MutexDoWorkIsRunning.unlock();

}


unsigned __int64 MainLogic::GetCurrentTimeStamp()
{
	auto epoch = std::chrono::steady_clock::now().time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
	unsigned __int64   CurrentTimeStamp = value.count();
	return CurrentTimeStamp;
}


bool MainLogic::SetCurrentMeasuringResults()
{
	ImageLineInformation combined_top, combined_bottom;
	unsigned __int64 transmission_offset_top, transmission_offset_bottom, transmission_end_top, transmission_end_bottom;
	double mm_per_us;// , speedInMPerMin = line_speed();
	unsigned __int64   current_time             = GetCurrentTimeStamp();
	double distanceTop                          = GetMainAppPrintCheck()->GetTransmissionDistanceCameraTopPLCInMM();// 500; // Werte 500mm Nach Mitte Kamerasystem Ÿbertragen (Weg konfigurierbar machen)
	double distanceBot                          = GetMainAppPrintCheck()->GetTransmissionDistanceCameraBotPLCInMM();
	double distance_camera_offset               = GetMainAppPrintCheck()->GetCameraCenterOffsetInMM();// 26.5; // Kameras sind jeweils +-26.5mm aus der Mitte. Oben/Unten prŸfen!!!
	double distance_camera_top;
	double distance_camera_bottom;  
	bool rv = false;
	
	

	if (GetMainAppPrintCheck()->GetTopCameraIsFirst())
	{//zweite Auslieferung System Zelle 1
		distance_camera_top     = distanceTop + distance_camera_offset;
		distance_camera_bottom  = distanceBot - distance_camera_offset;
	}
	else
	{
		distance_camera_top     = distanceTop - distance_camera_offset;
		distance_camera_bottom  = distanceBot + distance_camera_offset;
	}
	mm_per_us = m_CurrentLineSpeed * 1000.0 / (60.0 * 1000000);
	if (mm_per_us > 0.0)
	{
		transmission_offset_top    = (unsigned __int64)round(distance_camera_top / mm_per_us);
		transmission_offset_bottom = (unsigned __int64)round(distance_camera_bottom / mm_per_us);
		transmission_end_top       = current_time - transmission_offset_top;
		transmission_end_bottom    = current_time - transmission_offset_bottom;
		m_MutexLinesTop.lock();
		for (QList<ImageLineInformation>::Iterator it = m_LinesTop.begin(); it != m_LinesTop.end();)
		{
			if (it->m_TimeStampInMuSec <= transmission_end_top)
			{
				combined_top.tube_start     |= it->tube_start;
				combined_top.tube_end       |= it->tube_end;
				combined_top.tube_found     |= it->tube_found;

				combined_top.error_font     |= it->error_font;
				combined_top.error_position |= it->error_position;
				combined_top.error_line     |= it->error_line;
				combined_top.error_height   |= it->error_height;
				combined_top.error_width    |= it->error_width;
				combined_top.no_format      |= it->no_format;
				it = m_LinesTop.erase(it);
			}
			else
			{
				++it;
			}
     	}
		m_MutexLinesTop.unlock();
		m_MutexLinesBottom.lock();
		for (QList<ImageLineInformation>::Iterator it = m_LinesBottom.begin(); it != m_LinesBottom.end(); )
		{
			if (it->m_TimeStampInMuSec <= transmission_end_bottom)
			{
				combined_bottom.tube_start     |= it->tube_start;
				combined_bottom.tube_end       |= it->tube_end;
				combined_bottom.tube_found     |= it->tube_found;

				combined_bottom.error_font     |= it->error_font;
				combined_bottom.error_position |= it->error_position;
				combined_bottom.error_line     |= it->error_line;
				combined_bottom.error_height   |= it->error_height;
				combined_bottom.error_width    |= it->error_width;
				combined_bottom.no_format      |= it->no_format;
				it = m_LinesBottom.erase(it);
			}
			else
			{
				++it;
			}
		}
		m_MutexLinesBottom.unlock();
		set_tube_detected(combined_top.tube_found || combined_bottom.tube_found);

		set_cam_top_font(combined_top.error_font);
		set_cam_top_position(combined_top.error_position);
		set_cam_top_line(combined_top.error_line);
		set_cam_top_font_height(combined_top.error_height);
		set_cam_top_font_width(combined_top.error_width);
		set_cam_top_no_format(combined_top.no_format);

		set_cam_bottom_font(combined_bottom.error_font);
		set_cam_bottom_position(combined_bottom.error_position);
		set_cam_bottom_line(combined_bottom.error_line);
		set_cam_bottom_font_height(combined_bottom.error_height);
		set_cam_bottom_font_width(combined_bottom.error_width); 
		set_cam_bottom_no_format(combined_bottom.no_format);

        if (!GetMainAppPrintCheck()->DisableDebugInfoMeasureResults())
        {
            if (!GetMainAppPrintCheck()->ShowDebugInfoOnlyError())
            {
                if (!combined_top.tube_found || combined_top.error_font || combined_top.error_position || combined_top.error_line || combined_top.error_height || combined_top.error_width || combined_top.no_format)
                    qWarning() << "(PLC top result)" << "active:" << inspection_active() << inspection_disable() << " tube ok:" << (int)(combined_top.tube_found) << "start:" << tube_start() << "stop:" << tube_stop() << " font:" << (int)combined_top.error_font << " pos:" << (int)combined_top.error_position << " line:" << (int)combined_top.error_line << " h:" << (int)combined_top.error_height << " w:" << (int)combined_top.error_width << " format:" << (int)combined_top.no_format;
                else
                    qDebug() << "(PLC top result)" << "active:" << inspection_active() << inspection_disable() << " tube ok:" << (int)(combined_top.tube_found) << "start:" << tube_start() << "stop:" << tube_stop() << " font:" << (int)combined_top.error_font << " pos:" << (int)combined_top.error_position << " line:" << (int)combined_top.error_line << " h:" << (int)combined_top.error_height << " w:" << (int)combined_top.error_width << " format:" << (int)combined_top.no_format;
                
                if (!combined_bottom.tube_found || combined_bottom.error_font || combined_bottom.error_position || combined_bottom.error_line || combined_bottom.error_height || combined_bottom.error_width || combined_bottom.no_format)
                    qWarning() << "(PLC bot result)" << "active:" << inspection_active() << inspection_disable() << " tube ok:" << (int)(combined_bottom.tube_found) << "start:" << tube_start() << "stop:" << tube_stop() << " font:" << (int)combined_bottom.error_font << " pos:" << (int)combined_bottom.error_position << " line:" << (int)combined_bottom.error_line << " h:" << (int)combined_bottom.error_height << " w:" << (int)combined_bottom.error_width << " format:" << (int)combined_bottom.no_format;
                else
                    qDebug() << "(PLC bot result)" << "active:" << inspection_active() << inspection_disable() << " tube ok:" << (int)(combined_bottom.tube_found) << "start:" << tube_start() << "stop:" << tube_stop() << " font:" << (int)combined_bottom.error_font << " pos:" << (int)combined_bottom.error_position << " line:" << (int)combined_bottom.error_line << " h:" << (int)combined_bottom.error_height << " w:" << (int)combined_bottom.error_width << " format:" << (int)combined_bottom.no_format;
            }
            else
            {
                if(!combined_top.tube_found || combined_top.error_font || combined_top.error_position || combined_top.error_line || combined_top.error_height || combined_top.error_width || combined_top.no_format)
                    qWarning() << "(PLC top result)" << "active:" << inspection_active() << inspection_disable() << " tube ok:" << (int)(combined_top.tube_found) << "start:" << tube_start() << "stop:" << tube_stop() << " font:" << (int)combined_top.error_font << " pos:" << (int)combined_top.error_position << " line:" << (int)combined_top.error_line << " h:" << (int)combined_top.error_height << " w:" << (int)combined_top.error_width << " format:" << (int)combined_top.no_format;
                if (!combined_bottom.tube_found || combined_bottom.error_font || combined_bottom.error_position || combined_bottom.error_line || combined_bottom.error_height || combined_bottom.error_width || combined_bottom.no_format)
                    qWarning() << "(PLC bot result)" << "active:" << inspection_active() << inspection_disable() << " tube ok:" << (int)(combined_bottom.tube_found) << "start:" << tube_start() << "stop:" << tube_stop() << " font:" << (int)combined_bottom.error_font << " pos:" << (int)combined_bottom.error_position << " line:" << (int)combined_bottom.error_line << " h:" << (int)combined_bottom.error_height << " w:" << (int)combined_bottom.error_width << " format:" << (int)combined_bottom.no_format;

            }
        }
	}
	else
	{//Speed is zero
		ClearResults();
	}
	return rv;
}


void MainLogic::ClearResults()
{
	m_MutexLinesTop.lock();
	ClearResultsTop();
	m_MutexLinesTop.unlock();

	m_MutexLinesBottom.lock();
	ClearResultsBot();
	m_MutexLinesBottom.unlock();
}


void MainLogic::ClearResultsTop()
{
	for (auto it = m_LinesTop.begin(); it != m_LinesTop.end();)
		it = m_LinesTop.erase(it);
}


void MainLogic::ClearResultsBot()
{
	for (auto it = m_LinesBottom.begin(); it != m_LinesBottom.end();)
		it = m_LinesBottom.erase(it);
}

//1Meter entspricht ca. 18000 Bildzeilen   1000mm/(0.056mm/pix)  0.056 ist aktuelle Pixelsize
void MainLogic::AppendLineInformation(QList<ImageLineInformation> &lines, int CameraIndex)
{
	//if (m_IsDoWorkRunning)
	//{
		if (CameraIndex == CAMERA_TOP_INDEX)
		{
			m_MutexLinesTop.lock();
			if (m_LinesTop.count() > 180000)
				ClearResultsTop();//Hier Annahme DoWork läuft nicht oder Inspection Slot von der SPS kommt nicht, dann lösche die Daten um ein Überlauf zu verhindern 
			for (int i = 0; i < lines.size(); ++i)
				m_LinesTop.push_back(lines.at(i));
			m_MutexLinesTop.unlock();
		}
		else
		{
			m_MutexLinesBottom.lock();
			if (m_LinesBottom.count() > 180000)
				ClearResultsBot();//Hier Annahme DoWork läuft nicht oder Inspection Slot von der SPS kommt nicht, dann lösche die Daten um ein Überlauf zu verhindern 
			for (int i = 0; i < lines.size(); ++i)
				m_LinesBottom.push_back(lines.at(i));
			m_MutexLinesBottom.unlock();
		}
	//}
	
}


void MainLogic::setCurrentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState)
{ 
	m_MachineState = machineState; 
	m_DiagState    = diagState;
}


void MainLogic::watchdog()
{
	/*if (m_JobID == 11)
		m_JobID = 12;
	else
		m_JobID = 11;
	emit SignalLoadJob(m_JobID);
	*/
	if (admin()) 
	{
        if (m_AccessLevel != kAccessLevelAdmin)
		{
			m_AccessLevel = kAccessLevelAdmin;
            emit valueChanged("AccessLevel", kAccessLevelAdmin);
        }
    }
	else
	{
		if (service())
		{
			if (m_AccessLevel != kAccessLevelService)
			{
				m_AccessLevel = kAccessLevelService;
				emit valueChanged("AccessLevel", kAccessLevelService);
			}
		}
		else
		{
			if (m_AccessLevel >= kAccessLevelService)
			{
				m_AccessLevel = kAccessLevelGuest;
				emit valueChanged("AccessLevel", kAccessLevelGuest);
			}
		}
	}

	if (job_id() != m_JobID)
	{
        emit SignalLoadJob(job_id());
        m_JobID = job_id();
    }

	if (inspection_active() != m_InspectionActive)
	{
		emit SignalInspectionActive(inspection_active());
		m_InspectionActive = inspection_active();
	}
	
	if (line_speed() != m_CurrentLineSpeed)
	{
		emit SignalCurrentSpeed(line_speed());
		m_CurrentLineSpeed = line_speed();
	}
	
	if (m_TubeEndReached)
	{
		emit SignalTubeEndReached();
		m_TubeEndReached = false;
	}
	
	if (m_ResetFault)
	{
		emit SignalResetFault();
		m_ResetFault = false;
	}
}
