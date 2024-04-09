#include "WidgetEditReferenceImageData.h"
#include "MainAppPrintCheck.h"
#include "ImageData.h"
#include "ProductData.h"
#include "bmessagebox.h"
#include "WidgetEditCameraROI.h"
#include <popupdialog.h>
#include "MeasureTaskDetectHose.h"
#include "LiveImageView.h"


WidgetEditReferenceImageData::WidgetEditReferenceImageData(MainAppPrintCheck *pParent,int CameraIndex) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
, m_WindowSetup(false)
, m_PopupDialogEditCameraROI(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pParent;
	m_CameraIndex       = CameraIndex;

	connect(ui.pushButtonCheckNewReferenceFromCamera,    &QPushButton::clicked, this, &WidgetEditReferenceImageData::SlotCheckNewReference);
	connect(ui.pushButtonSaveAllReferenceData,           &QPushButton::clicked, this, &WidgetEditReferenceImageData::SlotSaveAllReferenceData);
	connect(ui.pushButtonDeleteSelectedRect,             &QPushButton::clicked, this, &WidgetEditReferenceImageData::SlotDeleteSelectedRect);
	connect(ui.pushButtonAddNewRect,                     &QPushButton::clicked, this, &WidgetEditReferenceImageData::SlotAddNewRect);
	connect(ui.pushButtonLoadReferenceFromDisk,          &QPushButton::clicked, this, &WidgetEditReferenceImageData::SlotReloadReferenceImage);
	connect(ui.pushButtonEditCameraROI,                  &QPushButton::clicked, this, &WidgetEditReferenceImageData::SlotCenterAllMeasureWidows);

	connect(ui.doubleSpinBoxSelectedWindowXPosition,     &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceImageData::SlotROIXPosChanged);
	connect(ui.doubleSpinBoxSelectedWindowYPosition,     &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceImageData::SlotROIYPosChanged);
	connect(ui.doubleSpinBoxSelectedWindowWidth,         &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceImageData::SlotROIWidthChanged);
	connect(ui.doubleSpinBoxSelectedWindowHeight,        &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceImageData::SlotROIHeightChanged);

	connect(ui.checkBoxShowRuler, &QCheckBox::stateChanged, this, &WidgetEditReferenceImageData::SlotShowRuler);

	m_MessageBox = new BMessageBox(QMessageBox::Warning, tr("Save New Reference Data, Please Wait..!"), tr("Save New Ref."));

	ui.frameDashBoardTitleChangeSelectedWindow->hide();
	ui.frameDashBoardROIMeasureXPosition->hide();
	ui.frameDashBoardROIMeasureYPosition->hide();
	ui.frameDashBoardROIMeasureWidth->hide();
	ui.frameDashBoardROIMeasureHeight->hide();
}


WidgetEditReferenceImageData::~WidgetEditReferenceImageData()
{
}


void WidgetEditReferenceImageData::SlotCenterAllMeasureWidows()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex) && GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetReferenceImageView())
	{
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetReferenceImageView()->YCenterAllMeasureWindow();// SetMeasureRulerIsActive(false);
	}
}


void WidgetEditReferenceImageData::SlotROIXPosChanged()
{

}

void WidgetEditReferenceImageData::SlotROIYPosChanged()
{

}

void WidgetEditReferenceImageData::SlotROIWidthChanged()
{
	/*if (m_WindowSetup && GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX) && GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX)->GetFormatImageView())
	{
		int NewWidth = static_cast<int>(ui.doubleSpinBoxSelectedWindowWidth->value());
		QRectF NewROIRect, SelectedROI;

		SelectedROI = GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX)->GetFormatImageView()->GetSelectedRect();
		if (SelectedROI.width() > 0 && SelectedROI.height() > 0)
		{
			if ((static_cast<int>(ui.doubleSpinBoxSelectedWindowXPosition->value()) + NewWidth) >= GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX)->GetImageWidth())
			{
				NewWidth = GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX)->GetImageWidth() - static_cast<int>(ui.doubleSpinBoxSelectedWindowXPosition->value()) - 1;
				disconnect(ui.doubleSpinBoxSelectedWindowWidth, &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceData::SlotROIWidthChangedCameraTop);
				ui.doubleSpinBoxSelectedWindowWidth->setValue(NewWidth);
				connect(ui.doubleSpinBoxSelectedWindowWidth, &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceData::SlotROIWidthChangedCameraTop);
			}

			if (NewWidth < MINIMUM_ROI_SIZE_IN_PIXEL)
			{
				NewWidth = MINIMUM_ROI_SIZE_IN_PIXEL;
				disconnect(ui.doubleSpinBoxSelectedWindowWidth, &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceData::SlotROIWidthChangedCameraTop);
				ui.doubleSpinBoxSelectedWindowWidth->setValue(NewWidth);
				connect(ui.doubleSpinBoxSelectedWindowWidth, &QDoubleSpinBox::editingFinished, this, &WidgetEditReferenceData::SlotROIWidthChangedCameraTop);
			}

			NewROIRect.setX(SelectedROI.x());
			NewROIRect.setY(SelectedROI.y());
			NewROIRect.setWidth(NewWidth);
			NewROIRect.setHeight(SelectedROI.height());

			//GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX)->SetMeasureWindowRect(ROI_ID_MEASURE_SPEED, NewROIRectMeasureSpeed);
			//GetMainAppPrintCheck()->GetImageData(CAMERA_TOP_INDEX)->GetFormatImageView()->DrawMeasureWindow(ROI_ID_MEASURE_SPEED);
		}
	}
	*/
}

void WidgetEditReferenceImageData::SlotROIHeightChanged()
{

}


void WidgetEditReferenceImageData::SlotReloadReferenceImage()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		if (!GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->IsResumeMeasuring())
		{
			GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->LoadAndShowReferenceImageFromDisk();
		}
	}
}


void WidgetEditReferenceImageData::ShowSelectedRectKoordinates(QRectF &CurrentRect)
{
	m_WindowSetup = false;
	ui.doubleSpinBoxSelectedWindowXPosition->setValue(CurrentRect.x());
	ui.doubleSpinBoxSelectedWindowYPosition->setValue(CurrentRect.y());
	ui.doubleSpinBoxSelectedWindowWidth->setValue(CurrentRect.width());
	ui.doubleSpinBoxSelectedWindowHeight->setValue(CurrentRect.height());
	m_WindowSetup = true;
}



void WidgetEditReferenceImageData::SlotAddNewRect()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		InspectionWindow *pInspectionWindow = new InspectionWindow();
		
		double RectHeight,RectWidth;
		int    ImageWidth,ImageHeight;
		double PixelSize = GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetPixelSize();
		
		RectHeight = 50.0;
		RectWidth  = 150.0;
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetMeasureTaskDetectHose()->GetReferenceSize(ImageWidth, ImageHeight);
		if (PixelSize > 0.0)
		{
				RectHeight = GetMainAppPrintCheck()->GetDefaultBlockHeightInMM() / PixelSize;
				RectWidth  = GetMainAppPrintCheck()->GetDefaultBlockWidthInMM() / PixelSize;
		}
		pInspectionWindow->m_ReferenceRect.setX(ImageWidth/2.0);
		pInspectionWindow->m_ReferenceRect.setY(ImageHeight/2.0 - RectHeight*2);
		pInspectionWindow->m_ReferenceRect.setWidth(RectWidth);
		pInspectionWindow->m_ReferenceRect.setHeight(RectHeight);
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->AddNewInspectionRect(pInspectionWindow);
	}
}


void WidgetEditReferenceImageData::SlotCheckNewReference()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->SetCheckNewReference(true);
	}
}


void WidgetEditReferenceImageData::SlotSaveAllReferenceData()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->SetEventGenerateReferenceData();
	}
}


void WidgetEditReferenceImageData::ShowInfoBoxStatusGeneraterefData(const QString ErrorMsg)
{
	if(ErrorMsg.isEmpty())
	   BMessageBox::information(this, tr("Save Reference Data!"), tr("Generate Reference Data Successful"), QMessageBox::Ok);
	else
	   BMessageBox::critical(this, tr("Save Reference Data!"), ErrorMsg, QMessageBox::Ok);
}


void WidgetEditReferenceImageData::SlotDeleteSelectedRect()
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->DeleteSelectedInspectionWindow();
	}
}


void WidgetEditReferenceImageData::AddReferenceImageWidget(QWidget *w)
{
	ui.ReferenceImage->layout()->addWidget(w);
}


void WidgetEditReferenceImageData::showEvent(QShowEvent *event)
{
	m_WindowSetup = false;
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		if (!GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->IsResumeMeasuring())
		{
			GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->LoadAndShowReferenceImageFromDisk();
		}

    	ProductData *pProductData=GetMainAppPrintCheck()->GetCurrentProductData();
		if (pProductData)
		{
			if (m_CameraIndex == CAMERA_TOP_INDEX)
				ui.labelDashBoardReference->setText(tr("Camera Top Reference Data (%1)").arg(pProductData->m_CameraTopType));
			else
				ui.labelDashBoardReference->setText(tr("Camera Bot Reference Data (%1)").arg(pProductData->m_CameraBotType));
		}
		ui.checkBoxShowRuler->setCheckState(Qt::Unchecked);
    }
	m_WindowSetup = true;
}


void WidgetEditReferenceImageData::SlotShowRuler(int State)
{
	if (m_WindowSetup)
	{
		if (State == Qt::Checked)
		{
			if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex) && GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetReferenceImageView())
			{
				GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetReferenceImageView()->SetMeasureRulerIsActive(true);
			}
		}
		else
		{
			if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex) && GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetReferenceImageView())
			{
				GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->GetReferenceImageView()->SetMeasureRulerIsActive(false);
			}
		}
	}
}


void WidgetEditReferenceImageData::hideEvent(QHideEvent *event)
{
	if (GetMainAppPrintCheck() && GetMainAppPrintCheck()->GetImageData(m_CameraIndex))
	{
		if (!GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->IsResumeMeasuring())
		{
			GetMainAppPrintCheck()->GetImageData(m_CameraIndex)->RemoveNotValidInspectionWindow();
		}
	}
}

