#include "bremsschlauchkontrolleplugin.h"
#include <QtCore>
#include <QtWidgets>
#include "MainAppPrintCheck.h"
#include <plugin.h>
#include <interfaces.h>
//#include "halconcpp/HalconCpp.h"


BremsSchlauchKontrollePlugin::BremsSchlauchKontrollePlugin(QObject *parent) : Plugin(parent)
, m_MainAppPrintCheck(NULL)//Hauptanwendung(Main Class) hier werden alle Instanzen gebildet die zu der Anwendung gehören
, m_HideWidgetMeasureIsRunning(true)//Wenn Messung gestartet(Produktion an) wird dieses Flag auf false gesetzt und bestimmte Widgets sind dann nicht mehr sichtbar 
, m_EnableShowProductWindow(false)//Wenn auf Einrichten geschaltet wird der Wert auf true gesetzt und der ProduktDialog kann dann geöffnet werden
, m_CurrentMaschineState(MachineState::Starting)//Aktueller Maschinenstatus
{
	qRegisterMetaType<ImageMetaData>();
	//read camera parameter from registry
	m_CameraIDTop = GetPreference("CameraIDTop").toString();
	if (m_CameraIDTop.isEmpty())
	{
		m_CameraIDTop = "12180307";//Zuordnugsnummer der Kamera oben
		SetPreference(QString("CameraIDTop"), QVariant(m_CameraIDTop));
	}
	m_CameraIDBot = GetPreference("CameraIDBot").toString();
	if (m_CameraIDBot.isEmpty())
	{
		m_CameraIDBot = "12181601";//Zuordnugsnummer der Kamera unten
		SetPreference(QString("CameraIDBot"), QVariant(m_CameraIDBot));
	}
	for (int index = 0; index < NUMBER_CAMERAS; index++)
	{//Kameraparameter von der Registry in die Klassenvariabeln
		QString CameraName;
		if (index == CAMERA_TOP_INDEX)
			CameraName = CAMERA_TOP_DIR_NAME;
		else
			CameraName = CAMERA_BOT_DIR_NAME;
		m_CameraWidthInPixel[index] = GetPreference(QString("%1/CameraWidthInPixel").arg(CameraName)).toInt();
		if (m_CameraWidthInPixel[index] == 0)
			m_CameraWidthInPixel[index] = 1024;
		SetPreference(QString("%1/CameraWidthInPixel").arg(CameraName), QVariant(m_CameraWidthInPixel[index]));
		m_CameraHeightInPixel[index] = GetPreference(QString("%1/CameraHeightInPixel").arg(CameraName)).toInt();
		if (m_CameraHeightInPixel[index] == 0)
			m_CameraHeightInPixel[index] = 1024;
		SetPreference(QString("%1/CameraHeightInPixel").arg(CameraName), QVariant(m_CameraHeightInPixel[index]));
		m_BinningHorizontal[index] = GetPreference(QString("%1/BinningHorizontal").arg(CameraName)).toInt();
		if (m_BinningHorizontal[index] == 0)
			m_BinningHorizontal[index] = 2;
		SetPreference(QString("%1/BinningHorizontal").arg(CameraName), QVariant(m_BinningHorizontal[index]));
		m_BinningVertical[index] = GetPreference(QString("%1/BinningVertical").arg(CameraName)).toInt();
		if (m_BinningVertical[index] == 0)
			m_BinningVertical[index] = 1;
		SetPreference(QString("%1/BinningVertical").arg(CameraName), QVariant(m_BinningVertical[index]));
		m_AcquisitionLineRate[index] = GetPreference(QString("%1/AcquisitionLineRate").arg(CameraName)).toInt();
		if (m_AcquisitionLineRate[index] == 0)
			m_AcquisitionLineRate[index] = 7500;
		SetPreference(QString("%1/AcquisitionLineRate").arg(CameraName), QVariant(m_AcquisitionLineRate[index]));
		m_ExposureTime[index] = GetPreference(QString("%1/ExposureTime").arg(CameraName)).toInt();
		if (m_ExposureTime[index] == 0)
			m_ExposureTime[index] = 100;
		SetPreference(QString("%1/ExposureTime").arg(CameraName),QVariant(m_ExposureTime[index]));
		m_HeartbeatTimeout[index] = GetPreference(QString("%1/HeartbeatTimeout").arg(CameraName)).toInt();
		if (m_HeartbeatTimeout[index] == 0)
			m_HeartbeatTimeout[index] = 12000;
		SetPreference(QString("%1/HeartbeatTimeout").arg(CameraName), QVariant(m_HeartbeatTimeout[index]));

	}
	m_MainAppPrintCheck                   = new MainAppPrintCheck(this);//Anlegen der Hauptinstanz
	//Pointer der einzelene GUI-Elemente, werden benötigt um die Elemente bei unter schiedlichen Benuzerlevel sichtbar oder unsichtbar zu schalten (siehe BremsSchlauchKontrollePlugin::mainWidget(const int idx))
	m_WidgetEditReferenceData             = GetMainAppPrintCheck()->GetWidgetEditReferenceData();
	m_WidgetEditProductData               = GetMainAppPrintCheck()->GetWidgetEditProductData();
	m_MainGUIPrintCheck                   = GetMainAppPrintCheck()->GetMainGUIPrintCheck();
	m_WidgetEditCustomSettings            = GetMainAppPrintCheck()->GetWidgetEditCustomSettings();
	m_WidgetEditGeneralSettings           = GetMainAppPrintCheck()->GetWidgetEditGeneralSettings();
	m_WidgetEditMeasureParameterCameraTop = GetMainAppPrintCheck()->GetWidgetEditMeasureParameterCameraTop();
    m_WidgetEditMeasureParameterCameraBot = GetMainAppPrintCheck()->GetWidgetEditMeasureParameterCameraBot();
}


BremsSchlauchKontrollePlugin::~BremsSchlauchKontrollePlugin()
{
	if (GetMainAppPrintCheck())
	{
		delete m_MainAppPrintCheck;
		m_MainAppPrintCheck = NULL;
    }
}

//not in use
void BremsSchlauchKontrollePlugin::SetCameraDeviceWidthInPixel(int set,int index)
{
	m_CameraWidthInPixel[index] =set;
	QString CameraName;
	if (index == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_BOT_DIR_NAME;
	SetPreference(QString("%1/CameraWidthInPixel").arg(CameraName), QVariant(set));
}

//not in use
void BremsSchlauchKontrollePlugin::SetCameraDeviceHeightInPixel(int set, int index)
{
	m_CameraHeightInPixel[index] = set;
	QString CameraName;
	if (index == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_BOT_DIR_NAME;
	SetPreference(QString("%1/CameraHeightInPixel").arg(CameraName), QVariant(set));
}

//not in use
void BremsSchlauchKontrollePlugin::SetBinningHorizontal(int set, int index)
{
	m_BinningHorizontal[index] = set;
	QString CameraName;
	if (index == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_BOT_DIR_NAME;
	SetPreference(QString("%1/BinningHorizontal").arg(CameraName), QVariant(set));
}

//not in use
void BremsSchlauchKontrollePlugin::SetBinningVertical(int set, int index)
{
	m_BinningVertical[index] = set;
	QString CameraName;
	if (index == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_BOT_DIR_NAME;
	SetPreference(QString("%1/BinningVertical").arg(CameraName), QVariant(set));
}

//Is called when speed is changed by PLC
void BremsSchlauchKontrollePlugin::SetAcquisitionLineRate(int set, int index)
{
	m_AcquisitionLineRate[index] = set;
	QString cameraId,CameraName,LogText;
	if (index == CAMERA_TOP_INDEX)
	{
		CameraName = CAMERA_TOP_DIR_NAME;
		cameraId   = m_CameraIDTop;
	}
	else
	{
		CameraName = CAMERA_BOT_DIR_NAME;
		cameraId   = m_CameraIDBot;
	}
	SetPreference(QString("%1/AcquisitionLineRate").arg(CameraName), QVariant(set));//wert speichern

	emit valueChanged(QString("%1/AcquisitionLineRate").arg(cameraId), m_AcquisitionLineRate[index]);//wert zur Kamera

	LogText = "AcquisitionLineRate Changed: " + CameraName + QString(" [1/sec] :%1").arg(m_AcquisitionLineRate[index]);

	if(GetMainAppPrintCheck())
	   GetMainAppPrintCheck()->WriteLogFile(LogText,QString("CameraAcquisitionLineRate.txt"));
}

//not in use
void BremsSchlauchKontrollePlugin::SetExposureTime(int set, int index)
{
	m_ExposureTime[index] = set;
	QString CameraName;
	if (index == CAMERA_TOP_INDEX)
		CameraName = CAMERA_TOP_DIR_NAME;
	else
		CameraName = CAMERA_TOP_DIR_NAME;
	SetPreference(QString("%1/ExposureTime").arg(CameraName), QVariant(set));
}

//Öffnet den Produktdialog
void BremsSchlauchKontrollePlugin::showProductWindow()
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->OpenProductDialog();
}

//Anzeigen unterschiedlicher Fehlertexte enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
void BremsSchlauchKontrollePlugin::SetMessage(const QString &message, QtMsgType MsgType)
{
	QList<PluginInterface::Message> messages;

	messages << PluginInterface::Message(256, message, MsgType);
	PluginInterface::updateMessages(messages);
}

//Wird aufgerufen wenn Start Messung(Produktion An set == true) oder Stop Messung(Einrichtbetrieb set == false)
void BremsSchlauchKontrollePlugin::HideWidgetMeasureIsRunning(bool set)
{
	m_HideWidgetMeasureIsRunning = set;
	emit valueChanged("updateUi", QVariant());//ruft dann mainWidget(const int idx) auf
}

//Wird von der obergeordnetet Instanz aufgerufen
QWidget *BremsSchlauchKontrollePlugin::mainWidget(const int idx) const
{
	switch (idx)
	{
	    case TAB_INDEX_MAIN_WIDGET_LIVE_IMAGE:
			 return (QWidget *)m_MainGUIPrintCheck;
		     break;
		case TAB_INDEX_MAIN_WIDGET_EDIT_REFERENCE:
			 if (m_HideWidgetMeasureIsRunning)
			 	return NULL;
			 else
			    return (QWidget *)m_WidgetEditReferenceData;
			 break;
		case TAB_INDEX_MAIN_WIDGET_EDIT_PRODUCT:
			 if (m_HideWidgetMeasureIsRunning)
				return NULL;
			 else
				return (QWidget *)m_WidgetEditProductData;
			 break;
		case TAB_INDEX_MAIN_WIDGET_CUSTOM_SETTINGS:
			if (m_HideWidgetMeasureIsRunning)
				return NULL;
			else
			    return (QWidget *)m_WidgetEditCustomSettings;
			break;
		case TAB_INDEX_MAIN_WIDGET_GENERAL_SETTINGS:
			if (m_HideWidgetMeasureIsRunning)
				return NULL;
			else
			    return (QWidget *)m_WidgetEditGeneralSettings;
			break;
		case TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_TOP:
			if (m_HideWidgetMeasureIsRunning)
				return NULL;
			else
				return (QWidget *)m_WidgetEditMeasureParameterCameraTop;
			break;
		case TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_BOT:
			if (m_HideWidgetMeasureIsRunning)
				return NULL;
			else
				return (QWidget *)m_WidgetEditMeasureParameterCameraBot;
			break;
		default:
			 return NULL;
			 break;
	}
}

//regelt den Zugriffslevel für die verschiedenen GUI-Widgets
int BremsSchlauchKontrollePlugin::requiredWidgetAccessLevel(const int idx) const 
{
	int rv;
	switch (idx)
	{
    
	case TAB_INDEX_MAIN_WIDGET_LIVE_IMAGE:
		 rv=kAccessLevelGuest;
		 break;
	case TAB_INDEX_MAIN_WIDGET_EDIT_REFERENCE:
		 rv=kAccessLevelService;
		 break;
	case TAB_INDEX_MAIN_WIDGET_EDIT_PRODUCT:
		 rv = kAccessLevelService;
		 break;
	case TAB_INDEX_MAIN_WIDGET_CUSTOM_SETTINGS:
		 rv = kAccessLevelAdmin;
		 break;
	case TAB_INDEX_MAIN_WIDGET_GENERAL_SETTINGS:
		 rv = kAccessLevelAdmin;
		 break;
	case TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_TOP:
		 rv = kAccessLevelBertram;
		 break;
	case TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_BOT:
		 rv = kAccessLevelBertram;
		 break;
	default:
		 rv = kAccessLevelSysOp;
		 break;
	}
#ifdef _DEBUG
	return kAccessLevelGuest;
#else
	return rv;
#endif
}

//Legt den Titel für die verschiedenen GUI-Elemnte fest
const QString BremsSchlauchKontrollePlugin::title(const int idx) const
{
	QString Name;
	switch (idx)
	{
	case TAB_INDEX_MAIN_WIDGET_LIVE_IMAGE:
		 Name = tr("Live Image");
		 break;
	case TAB_INDEX_MAIN_WIDGET_EDIT_REFERENCE:
		 Name = tr("Edit Reference.");
		 break;
	case TAB_INDEX_MAIN_WIDGET_EDIT_PRODUCT:
		 Name = tr("Edit Product");
		 break;
	case TAB_INDEX_MAIN_WIDGET_CUSTOM_SETTINGS:
		 Name = tr("Custom Settings");
		 break;
	case TAB_INDEX_MAIN_WIDGET_GENERAL_SETTINGS:
		 Name = tr("General Settings");
		 break;
	case TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_TOP:
		 Name = tr("Measure Top Settings");
		 break;
	case TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_BOT:
		 Name = tr("Measure Bot Settings");
		 break;
	default:
		 Name = tr("NoName");
		 break;
	}
	return Name;
}

//Entfernt alle Textnachrichen aus dem Fenster
void BremsSchlauchKontrollePlugin::reset()
{
	clearMessages();
}

//wird von der Übergeordneten Instanz aufgerufen wenn zwischen Einrichetn und Produktion gewechselt wird
void BremsSchlauchKontrollePlugin::requestMachineState(const PluginInterface::MachineState state)
{
	if (GetMainAppPrintCheck())
	{
		if (state == MachineState::Production)
		{
			GetMainAppPrintCheck()->SlotInspectionActive(true);//Aktiviert die Inspektion
		}
		else
		{
			GetMainAppPrintCheck()->SlotInspectionActive(false);//Stoppt die Inspektion
		}
	}
}

//Initialisierundsroutine wird einmalig beim Start aufgerufen
void BremsSchlauchKontrollePlugin::initialize()
{
    QHash<QString, QVariant> config;
    config["type"] = "device";
    config["logic"] = QVariant::fromValue(std::dynamic_pointer_cast<Logic>(GetMainAppPrintCheck()->GetPLCSharedPtr()));
    
    // configuration for values that will be received
    int in_slot = 1;
    int in_subslot = 1000;
    QList<QVariant> in_config;
    in_config << PNIO_DeviceValue("coordination", tr("Coordination"), QVariant::Int, in_slot, in_subslot, 0, 2);
    
    in_config << PNIO_DeviceValue("in_bit_0", tr("Bit 0"), QVariant::Bool, in_slot, in_subslot, 2, 0);
    in_config << PNIO_DeviceValue("admin", tr("Admin"), QVariant::Bool, in_slot, in_subslot, 2, 1);
    in_config << PNIO_DeviceValue("service", tr("Service"), QVariant::Bool, in_slot, in_subslot, 2, 2);
    in_config << PNIO_DeviceValue("in_bit_3", tr("Bit 3"), QVariant::Bool, in_slot, in_subslot, 2, 3);
    in_config << PNIO_DeviceValue("reset_fault", tr("Reset Fault"), QVariant::Bool, in_slot, in_subslot, 2, 4);
    in_config << PNIO_DeviceValue("in_bit_5", tr("Bit 5"), QVariant::Bool, in_slot, in_subslot, 2, 5);
    in_config << PNIO_DeviceValue("in_bit_6", tr("Bit 6"), QVariant::Bool, in_slot, in_subslot, 2, 6);
    in_config << PNIO_DeviceValue("in_bit_7", tr("Bit 7"), QVariant::Bool, in_slot, in_subslot, 2, 7);
    in_config << PNIO_DeviceValue("inspection_active", tr("Inspection active"), QVariant::Bool, in_slot, in_subslot, 3, 0);
    in_config << PNIO_DeviceValue("inspection_disable", tr("Inspection disabled"), QVariant::Bool, in_slot, in_subslot, 3, 1);
    in_config << PNIO_DeviceValue("in_bit_10", tr("Bit 10"), QVariant::Bool, in_slot, in_subslot, 3, 2);
    in_config << PNIO_DeviceValue("tube_stop", tr("Tube stop"), QVariant::Bool, in_slot, in_subslot, 3, 3);
    in_config << PNIO_DeviceValue("tube_start", tr("Tube start"), QVariant::Bool, in_slot, in_subslot, 3, 4);
    in_config << PNIO_DeviceValue("in_bit_13", tr("Bit 13"), QVariant::Bool, in_slot, in_subslot, 3, 5);
    in_config << PNIO_DeviceValue("in_bit_14", tr("Bit 14"), QVariant::Bool, in_slot, in_subslot, 3, 6);
    in_config << PNIO_DeviceValue("inspection_slot", tr("Inspection Slot"), QVariant::Bool, in_slot, in_subslot, 3, 7);
    
    in_config << PNIO_DeviceValue("job_id", tr("Job ID"), QVariant::Int, in_slot, in_subslot, 4, 2);
    in_config << PNIO_DeviceValue("line_speed", tr("Speed"), QVariant::Double, in_slot, in_subslot, 6);
    config["in_config"] = in_config;
    
    // configuration for values that will be sent
    QList<QVariant> out_config;
    int out_slot = 1;
    int out_subslot = 1001;
    out_config << PNIO_DeviceValue("ack_coordination", tr("ACK Coordination"), QVariant::Int, out_slot, out_subslot, 0, 2);
    
    out_config << PNIO_DeviceValue("software_started", tr("Software Started"), QVariant::Bool, out_slot, out_subslot, 2, 0);
    out_config << PNIO_DeviceValue("out_bit_1", tr("Bit 1"), QVariant::Bool, out_slot, out_subslot, 2, 1);
    out_config << PNIO_DeviceValue("out_bit_2", tr("Bit 2"), QVariant::Bool, out_slot, out_subslot, 2, 2);
    out_config << PNIO_DeviceValue("has_warning", tr("Warning"), QVariant::Bool, out_slot, out_subslot, 2, 3);
    out_config << PNIO_DeviceValue("has_fault", tr("Fault"), QVariant::Bool, out_slot, out_subslot, 2, 4);
    out_config << PNIO_DeviceValue("out_bit_5", tr("Bit 5"), QVariant::Bool, out_slot, out_subslot, 2, 5);
    out_config << PNIO_DeviceValue("out_bit_6", tr("Bit 6"), QVariant::Bool, out_slot, out_subslot, 2, 6);
    out_config << PNIO_DeviceValue("out_bit_7", tr("Bit 7"), QVariant::Bool, out_slot, out_subslot, 2, 7);
    out_config << PNIO_DeviceValue("inspection_is_active", tr("Inspection Active"), QVariant::Bool, out_slot, out_subslot, 3, 0);
    out_config << PNIO_DeviceValue("out_bit_9", tr("Bit 9"), QVariant::Bool, out_slot, out_subslot, 3, 1);
    out_config << PNIO_DeviceValue("out_bit_10", tr("Bit 10"), QVariant::Bool, out_slot, out_subslot, 3, 2);
    out_config << PNIO_DeviceValue("tube_end_detected", tr("Tube End Detected"), QVariant::Bool, out_slot, out_subslot, 3, 3);
    out_config << PNIO_DeviceValue("tube_start_detected", tr("Tube Start Detected"), QVariant::Bool, out_slot, out_subslot, 3, 4);
    out_config << PNIO_DeviceValue("tube_detected", tr("Tube Detected"), QVariant::Bool, out_slot, out_subslot, 3, 5);
    out_config << PNIO_DeviceValue("out_bit_14", tr("Bit 14"), QVariant::Bool, out_slot, out_subslot, 3, 6);
    out_config << PNIO_DeviceValue("out_bit_15", tr("Bit 15"), QVariant::Bool, out_slot, out_subslot, 3, 7);

    out_config << PNIO_DeviceValue("ack_job_id", tr("ACK Job ID"), QVariant::Int, out_slot, out_subslot, 4, 2);
    
    out_config << PNIO_DeviceValue("cam_top_font", tr("Camera Top Font Incorrect"), QVariant::Bool, out_slot, out_subslot, 6, 0);
    out_config << PNIO_DeviceValue("cam_top_position", tr("Camera Top Position Incorrect"), QVariant::Bool, out_slot, out_subslot, 6, 1);
    out_config << PNIO_DeviceValue("cam_top_line", tr("Camera Top Line Missing"), QVariant::Bool, out_slot, out_subslot, 6, 2);
    out_config << PNIO_DeviceValue("cam_top_font_height", tr("Camera Top Font Height Incorrect"), QVariant::Bool, out_slot, out_subslot, 6, 3);
    out_config << PNIO_DeviceValue("cam_top_font_width", tr("Camera Top Font Width Incorrect"), QVariant::Bool, out_slot, out_subslot, 6, 4);
    out_config << PNIO_DeviceValue("cam_top_bit_5", tr("Camera Top Bit 5"), QVariant::Bool, out_slot, out_subslot, 6, 5);
    out_config << PNIO_DeviceValue("cam_top_bit_6", tr("Camera Top Bit 6"), QVariant::Bool, out_slot, out_subslot, 6, 6);
    out_config << PNIO_DeviceValue("cam_top_no_format", tr("Camera Top No Format"), QVariant::Bool, out_slot, out_subslot, 6, 7);

    out_config << PNIO_DeviceValue("cam_bottom_font", tr("Camera Bottom Font Incorrect"), QVariant::Bool, out_slot, out_subslot, 7, 0);
    out_config << PNIO_DeviceValue("cam_bottom_position", tr("Camera Bottom Position Incorrect"), QVariant::Bool, out_slot, out_subslot, 7, 1);
    out_config << PNIO_DeviceValue("cam_bottom_line", tr("Camera Bottom Line Missing"), QVariant::Bool, out_slot, out_subslot, 7, 2);
    out_config << PNIO_DeviceValue("cam_bottom_font_height", tr("Camera Bottom Font Height Incorrect"), QVariant::Bool, out_slot, out_subslot, 7, 3);
    out_config << PNIO_DeviceValue("cam_bottom_font_width", tr("Camera Bottom Font Width Incorrect"), QVariant::Bool, out_slot, out_subslot, 7, 4);
    out_config << PNIO_DeviceValue("cam_bottom_bit_5", tr("Camera Bottom Bit 5"), QVariant::Bool, out_slot, out_subslot, 7, 5);
    out_config << PNIO_DeviceValue("cam_bottom_bit_6", tr("Camera Bottom Bit 6"), QVariant::Bool, out_slot, out_subslot, 7, 6);
    out_config << PNIO_DeviceValue("cam_bottom_no_format", tr("Camera Bottom No Format"), QVariant::Bool, out_slot, out_subslot, 7, 7);
    
    for (int i = 0; i < 8; ++i) {
        out_config << PNIO_DeviceValue(QString("test_job_id_bit_%1").arg(i), QString("test_job_id_bit_%1").arg(i), QVariant::Bool, out_slot, out_subslot, 14, i);
    }
    for (int i = 0; i < 8; ++i) {
        out_config << PNIO_DeviceValue(QString("test_job_id_bit_%1").arg(8 + i), QString("test_job_id_bit_%1").arg(8 + i), QVariant::Bool, out_slot, out_subslot, 15, i);
    }

    config["out_config"] = out_config;
    
    emit valueChanged("PNIO/configure", config);
	if(GetMainAppPrintCheck())
		GetMainAppPrintCheck()->StartImageAcquisition();
	SetCurrentMaschineState(MachineState::Setup);
}

//Bild von den Kameras zur Anwendung
void BremsSchlauchKontrollePlugin::consumeImage(const QString &cameraId, unsigned long frameNumber, int frameStatus, const cv::Mat &image)
{
    if (GetMainAppPrintCheck())
	{
		if (cameraId == m_CameraIDTop)
		{
			if(!GetMainAppPrintCheck()->IsSimulationCameraTopOn())
			    GetMainAppPrintCheck()->SetImageCameraTop(image, frameNumber, frameStatus);
		}
		else
		{
			if (cameraId == m_CameraIDBot)
			{
				if (!GetMainAppPrintCheck()->IsSimulationCameraBotOn())
				     GetMainAppPrintCheck()->SetImageCameraBot(image, frameNumber, frameStatus);
			}
		}
	}
}

//wird aufgerufen wenn Programm beendet wird
void BremsSchlauchKontrollePlugin::uninitialize()
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->FinishedMeasuringAndImageAcquisition();
}

//Wird aufgerufen wenn der Maschinestate in der GUI manuell geändert wird die Information wrd hier an die SPS weitergeletet
void BremsSchlauchKontrollePlugin::currentMachineState(const PluginInterface::MachineState machineState, const PluginInterface::DiagState diagState)
{
	if (GetMainAppPrintCheck())
		GetMainAppPrintCheck()->currentMachineState(machineState, diagState);//an die SPS
}


void BremsSchlauchKontrollePlugin::loadPreferences()
{
	
}

//Aktuelle Kameraparameter direkt an die Kamera
void BremsSchlauchKontrollePlugin::setValue(const QString &name, const QVariant &value)
{
    if (name.endsWith("/StatusCamera")) 
	{
        if (value.toInt() == 0)
		{
            QStringList parts = name.split("/", QString::SkipEmptyParts);
            QString cameraId = parts.at(0);
			int index = CAMERA_TOP_INDEX;
			if (m_CameraIDBot == cameraId)
				index = CAMERA_BOT_INDEX;

            emit valueChanged(QString("%1/StopCapturing").arg(cameraId), QVariant());
            emit valueChanged(QString("%1/GevHeartbeatTimeout").arg(cameraId), m_HeartbeatTimeout[index]);
            emit valueChanged(QString("%1/ExposureTime").arg(cameraId), m_ExposureTime[index]);
            emit valueChanged(QString("%1/AcquisitionLineRate").arg(cameraId), m_AcquisitionLineRate[index]);
            emit valueChanged(QString("%1/Height").arg(cameraId), m_CameraHeightInPixel[index] * m_BinningVertical[index]);
			emit valueChanged(QString("%1/Width").arg(cameraId),  m_CameraWidthInPixel[index]  * m_BinningHorizontal[index]);//da BinningHorizontal auf 2
			emit valueChanged(QString("%1/BinningHorizontal").arg(cameraId), m_BinningHorizontal[index]);
			emit valueChanged(QString("%1/BinningVertical").arg(cameraId), m_BinningVertical[index]);
            emit valueChanged(QString("%1/ReverseX").arg(cameraId), false); 
            emit valueChanged(QString("%1/TriggerMode").arg(cameraId), "Off");
            emit valueChanged(QString("%1/StartCapturing").arg(cameraId), QVariant());
        }
    } 
	else 
	{
		
#ifdef DEBUG
        qDebug() << this << "ignored setValue" << name << value;
#endif
    }
}


void BremsSchlauchKontrollePlugin::setValues(const QHash<QString, QVariant> &values)
{
#ifdef DEBUG
    qDebug() << this << "ignored setValues" << values;
#else
    Q_UNUSED(values);
#endif
}


void BremsSchlauchKontrollePlugin::SetPreference(const QString & preference, const QVariant & value)
{
	setPreference(preference, value);
}


QVariant BremsSchlauchKontrollePlugin::GetPreference(const QString & preference)
{
	return getPreference(preference);
}
