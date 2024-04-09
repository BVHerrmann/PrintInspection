#pragma once

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "halconcpp/HalconCpp.h"
#include "GlobalConst.h"


class CameraImageAndTimeStamps
{
public:
	int AddToHalconImage(int rows,int cols,unsigned char *data, bool FromCamera,QString &ErrorMsg)
	{
		int rv= ERROR_CODE_NO_ERROR;
		try
		{//Bild nach Halcon copieren
			m_CameraImage.GenImage1("byte", static_cast<Hlong>(cols), static_cast<Hlong>(rows), (void*)(data));//von Mat nach Halcon kopieren
			//GrayClosingRect(m_CameraImage, &m_CameraImage, 3, 3);
		}
		catch (HalconCpp::HException &exception)
		{//schwerwiegender Fehler
			ErrorMsg = QString("Error/License Error Input Image. In Funktion:%1 %2 %3").arg(exception.ErrorCode()).arg((const char *)exception.ProcName()).arg((const char *)exception.ErrorMessage());
			rv = ERROR_CODE_ANY_ERROR;
		}
		return rv;
	};
public:
	HalconCpp::HImage  m_CameraImage;
	unsigned __int64   m_TimeStampInMuSec;
};