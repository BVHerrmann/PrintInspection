#include "WidgetEditCameraROI.h"
#include "MainAppPrintCheck.h"
#include "qpushbutton.h"
#include "ProductData.h"



WidgetEditCameraROI::WidgetEditCameraROI(MainAppPrintCheck *pMainAppPrintCheck, int CameraIndex) : QWidget()//(QWidget *)(pMainAppPrintCheck->GetMainGUIPrintCheck()))//((QWidget *)(pMainAppPrintCheck->GetMainGUIPrintCheck()))
, m_MainAppPrintCheck(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pMainAppPrintCheck;
	m_CameraIndex       = CameraIndex;
}


WidgetEditCameraROI::~WidgetEditCameraROI()
{
}


void WidgetEditCameraROI::SlotApplySettings()
{
	if (GetMainAppPrintCheck())
	{
		ImageData *pImageData = GetMainAppPrintCheck()->GetImageData(m_CameraIndex);
		if (pImageData)
		{
			InspectionWindow *pInspectionWindowHoseDetection = pImageData->GetInspectionWindowHoseDetection();
			if (pInspectionWindowHoseDetection)
			{
				pInspectionWindowHoseDetection->m_ReferenceRect.setX(ui.doubleSpinBoxROIHoseDetectionXPosition->value());
				pInspectionWindowHoseDetection->m_ReferenceRect.setY(ui.doubleSpinBoxROIHoseDetectionYPosition->value());
				pInspectionWindowHoseDetection->m_ReferenceRect.setWidth(ui.doubleSpinBoxROIHoseDetectionWidth->value());
				pInspectionWindowHoseDetection->m_ReferenceRect.setHeight(ui.doubleSpinBoxROIHoseDetectionHeight->value());
				pInspectionWindowHoseDetection->m_ModelHeightReference = ui.doubleSpinBoxROIHoseDetectionHeight->value();

				QString ErrorMsg;
				ProductData *pCurrentProduct = GetMainAppPrintCheck()->GetCurrentProductData();
				if (pCurrentProduct)
				{
					pCurrentProduct->WriteProductData(ErrorMsg);
				}
			}
		}
	}
}

void WidgetEditCameraROI::SlotCancel()
{
	
}


void WidgetEditCameraROI::showEvent(QShowEvent *event)
{
	if (GetMainAppPrintCheck())
	{
		ImageData *pImageData = GetMainAppPrintCheck()->GetImageData(m_CameraIndex);
		if (pImageData)
		{
			InspectionWindow *pInspectionWindowHoseDetection = pImageData->GetInspectionWindowHoseDetection();
			if (pInspectionWindowHoseDetection)
			{
				ui.doubleSpinBoxROIHoseDetectionXPosition->setValue(pInspectionWindowHoseDetection->m_ReferenceRect.topLeft().x());
				ui.doubleSpinBoxROIHoseDetectionYPosition->setValue(pInspectionWindowHoseDetection->m_ReferenceRect.topLeft().y());
				ui.doubleSpinBoxROIHoseDetectionWidth->setValue(pInspectionWindowHoseDetection->m_ReferenceRect.width());
				ui.doubleSpinBoxROIHoseDetectionHeight->setValue(pInspectionWindowHoseDetection->m_ReferenceRect.height());
			}
		}
	}
}


PopupDialogEditCameraROI::PopupDialogEditCameraROI(MainAppPrintCheck *pMainAppPrintCheck, int CameraIndex, QWidget *parent) : PopupDialog(parent)
{
	WidgetEditCameraROI *pDialog = new WidgetEditCameraROI(pMainAppPrintCheck, CameraIndex);
	QBoxLayout *box = new QVBoxLayout();
	centralWidget()->setLayout(box);

	//setWindowTitle(tr("MeasureWindow Camera"));

	box->addWidget(pDialog);
	m_button_box = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
	//button_box->button(QDialogButtonBox::Apply)->setText(ApplyText);
	//button_box->button(QDialogButtonBox::Cancel)->setText(CancelText);
	connect(m_button_box, &QDialogButtonBox::clicked, [=](QAbstractButton *button)
	{
		switch (m_button_box->standardButton(button))
		{
		case QDialogButtonBox::Apply:
			pDialog->SlotApplySettings();
			this->close();
			break;
		case QDialogButtonBox::Cancel:
			this->close();
			break;
		default:
			break;
		}
	});
	box->addWidget(m_button_box);
}


void PopupDialogEditCameraROI::SetButtonText(QString &ApplyText, QString &CancelText)
{
	m_button_box->button(QDialogButtonBox::Apply)->setText(ApplyText);
	m_button_box->button(QDialogButtonBox::Cancel)->setText(CancelText);
	
}
