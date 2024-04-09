#include "SharedMemoryVideoData.h"
#include "GlobalConst.h"


SharedMemoryVideoData::SharedMemoryVideoData()
{
    m_EventIDNewDataInSharedMemory       = NULL;
    m_KeyName                            = "";
    m_CurrentSharedMemorySize            = 0;
	
}


SharedMemoryVideoData::~SharedMemoryVideoData()
{
	bool rv;
	if (isAttached())
		rv=detach();
	if (m_EventIDNewDataInSharedMemory)
		CloseHandle(m_EventIDNewDataInSharedMemory);
}


int SharedMemoryVideoData::OpenSharedMemory(QString &ErrorMsg)
{
	int rv = 0;
    if(!isAttached())
    {
        if(!attach())
        {
            rv=-1;
            ErrorMsg=tr("Server Program Is Not Active. Invalid Handle. Can Not Open Shared Memory Area!");
        }
    }
    return rv;
}


void SharedMemoryVideoData::CloseSharedMemory()
{
	bool rv;
	if (isAttached())
		rv=detach();
}


void SharedMemoryVideoData::SetKeyName(const QString &Name)
{
    m_KeyName=Name;
    setKey(m_KeyName);
    CreateNewEventHandle(m_KeyName); 
}


void SharedMemoryVideoData::CreateNewEventHandle(QString &Name)
{
    QString EventName="EVENT_";
    wchar_t *pWCharEventName=NULL;
    
    EventName=EventName+Name;
    pWCharEventName = new wchar_t[EventName.size()+1];
    EventName.toWCharArray(pWCharEventName);
    pWCharEventName[EventName.size()]=0;

    if(m_EventIDNewDataInSharedMemory)
        CloseHandle(m_EventIDNewDataInSharedMemory);
    m_EventIDNewDataInSharedMemory=CreateEventW(NULL,FALSE,FALSE,pWCharEventName);
    delete[] pWCharEventName;
}


unsigned char *SharedMemoryVideoData::GetSharedMemoryStartPointer()
{
    return (unsigned char *) data();
}


void SharedMemoryVideoData::SetEventNewData()
{
	SetEvent(m_EventIDNewDataInSharedMemory);
}





int SharedMemoryVideoData::CreateNew(unsigned __int64  SaredSize, QString &ErrorMsg)
{
	m_CurrentSharedMemorySize = SaredSize;
	QString ErrorText;
	if (!create(m_CurrentSharedMemorySize))
	{
		switch (error())
		{
		case QSharedMemory::NoError:
			ErrorText = "NoError";
			break;
		case QSharedMemory::PermissionDenied:
			ErrorText = "PermissionDenied";
			break;
		case QSharedMemory::InvalidSize:
			ErrorText = "InvalidSize";
			break;
		case QSharedMemory::KeyError:
			ErrorText = "KeyError";
			break;
		case QSharedMemory::AlreadyExists:
			ErrorText = "AlreadyExists";
			break;
		case QSharedMemory::NotFound:
			ErrorText = "NotFound";
			break;
		case QSharedMemory::LockError:
			ErrorText = "LockError";
			break;
		case QSharedMemory::OutOfResources:
			ErrorText = "OutOfResources";
			break;
		default:
			ErrorText = "QSharedMemory::UnknownError";
				break;

		};
		ErrorMsg = tr("Unable To Create Shared Memory KeyName:%1.  %2").arg(m_KeyName).arg(ErrorText);
		m_CurrentSharedMemorySize = 0;
		return ERROR_CODE_ANY_ERROR;
	}
	else
	{
		return ERROR_CODE_NO_ERROR;
	}
}



