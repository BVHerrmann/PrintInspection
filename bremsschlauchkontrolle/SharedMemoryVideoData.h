#ifndef SHARED_MEMORY_VIDEO_DATA_H
#define SHARED_MEMORY_VIDEO_DATA_H


#include <Qtcore>
#include "Windows.h"


class SharedMemoryVideoData : public QSharedMemory
{
	//Q_OBJECT
 public:
	 SharedMemoryVideoData();
	~SharedMemoryVideoData();
	int  OpenSharedMemory(QString &ErrorMsg);
	HANDLE  GetEventIDNewDataInSharedMemory(){return m_EventIDNewDataInSharedMemory;}
    int  CreateNew(unsigned __int64 size,QString &ErrorMsg);
    void SetKeyName(const QString &Name);
    void CreateNewEventHandle(QString &Name);
	QString GetKeyName(){return m_KeyName;}
    unsigned char *GetSharedMemoryStartPointer();
	void SetEventNewData();
	unsigned __int64    GetCurrentSharedMemorySize() { return m_CurrentSharedMemorySize; }
	void CloseSharedMemory();
	
 private:
	QString             m_KeyName;
    HANDLE              m_EventIDNewDataInSharedMemory;
  	unsigned __int64    m_CurrentSharedMemorySize;
	
};
#endif
