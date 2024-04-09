#include "InspectionRectDialog.h"
#include "ImageData.h"
#include "InspectionWindow.h"
#include "GlobalConst.h"



InspectionRectDialog::InspectionRectDialog(ImageData *pImageData, QWidget *parent) : QDialog(parent)
, m_ImageData(NULL)
, m_CurrentInspectionRectIndex(-1)
, m_DeleteButtonOn(false)
, m_SetupWindow(false)
{
	ui.setupUi(this);
	m_ImageData = pImageData;
	
	connect(ui.checkBoxUseOnlyLineCheck, &QCheckBox::stateChanged, this, &InspectionRectDialog::SlotLineCheckChanged);
	connect(ui.checkBoxDisableBlock,     &QCheckBox::stateChanged, this, &InspectionRectDialog::SlotEnableBlockChanged);
	m_SetupWindow = true;
}


InspectionRectDialog::~InspectionRectDialog()
{
}


void InspectionRectDialog::SlotLineCheckChanged(int state)
{
	if (m_SetupWindow)
	{
		if (state == Qt::Checked)
		{
			ShowLineCheckParameter(true);
			ui.frameDashBoardPrintProperty->hide();
			ui.frameDashBoardPrintTolerance->hide();
		}
		else
		{
			ShowLineCheckParameter(false);
			ui.frameDashBoardPrintProperty->show();
			ui.frameDashBoardPrintTolerance->show();
		}
	}
}

void InspectionRectDialog::ShowLineCheckParameter(bool show)
{
	if (show)
	{
		ui.doubleSpinBoxNumberLines->show();
		ui.labelNumberLines->show();
		ui.frameDashBoardBlockHeight->show();
		ui.frameDashBoardBlockWidth->show();

		InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(m_CurrentInspectionRectIndex);

		if (pInspectionWindow)
		{
			if (pInspectionWindow->m_ModelHeightReference == 0)
				pInspectionWindow->m_ModelHeightReference = 2.5 / GetImageData()->GetPixelSize();
			ui.doubleSpinBoxBlockHeight->setValue(pInspectionWindow->m_ModelHeightReference * GetImageData()->GetPixelSize());

			if (pInspectionWindow->m_ModelWidthReference == 0)
				pInspectionWindow->m_ModelWidthReference = 5.0 / GetImageData()->GetPixelSize();
			ui.doubleSpinBoxBlockWidth->setValue(pInspectionWindow->m_ModelWidthReference * GetImageData()->GetPixelSize());
		}
	}
	else
	{
		ui.doubleSpinBoxNumberLines->hide();
		ui.labelNumberLines->hide();
		ui.frameDashBoardBlockHeight->hide();
		ui.frameDashBoardBlockWidth->hide();
	}
}


void InspectionRectDialog::SlotEnableBlockChanged(int state)
{
	if (m_SetupWindow)
	{
		if (state == Qt::Checked)
		{
			ShowLineCheckParameter(false);
			ui.frameDashBoardPrintProperty->hide();
			ui.frameDashBoardPrintTolerance->hide();
			ui.checkBoxUseOnlyLineCheck->hide();
			ui.labelOnlyLineCheck->hide();
		}
		else
		{
			ui.checkBoxUseOnlyLineCheck->show();
			ui.labelOnlyLineCheck->show();
			if (ui.checkBoxUseOnlyLineCheck->checkState() == Qt::Checked)
			{
				ShowLineCheckParameter(true);
				ui.frameDashBoardPrintProperty->hide();
				ui.frameDashBoardPrintTolerance->hide();
			}
			else
			{
				ShowLineCheckParameter(false);
				ui.frameDashBoardPrintProperty->show();
				ui.frameDashBoardPrintTolerance->show();
			}
		}
	}
}


void InspectionRectDialog::SetInspectionRectID(int Index,bool DeleteButtonOn)
{
	m_SetupWindow = false;
	InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(Index);
	
	m_DeleteButtonOn=DeleteButtonOn;
	if (pInspectionWindow)
	{
		m_CurrentInspectionRectIndex = Index;
		QString TextID = QString("Blocknr:%1").arg(pInspectionWindow->m_InspectionWindowID);
		QString Name   = QString("%1").arg(pInspectionWindow->m_ModelName);
		QString ReferenceLocation = GetImageData()->GetReferenceLocation() + QString("/") + QString("Block%1").arg(pInspectionWindow->m_InspectionWindowID) + "/" + REFERENCE_MODEL_IMAGE_FILE_NAME;
		QFile RefSubImageFile(ReferenceLocation);

		if (pInspectionWindow->m_MeasureVarianteDiffImage)
		{
			ui.radioButtonPrintPropertyDynamic->setChecked(false);
			ui.radioButtonPrintPropertyStatic->setChecked(true);
		}
		else
		{
			ui.radioButtonPrintPropertyDynamic->setChecked(true);
			ui.radioButtonPrintPropertyStatic->setChecked(false);
		}
		ui.doubleSpinBoxNumberLines->setValue(pInspectionWindow->m_NumberHorizontalLines);
		ui.doubleSpinBoxBlockWidth->setValue(pInspectionWindow->m_ModelWidthReference * GetImageData()->GetPixelSize());
		ui.doubleSpinBoxBlockHeight->setValue(pInspectionWindow->m_ModelHeightReference * GetImageData()->GetPixelSize());
		ui.doubleSpinBoxPrintErrorTolInPercent->setValue(pInspectionWindow->m_PrintErrorTolInPercent);
		setWindowTitle(TextID);
		ui.lineEditInspectionWindowName->setText(Name);
		if (pInspectionWindow->m_EnableInspection)
		{
			ui.checkBoxDisableBlock->setCheckState(Qt::Unchecked);
		}
		else
		{
			ui.checkBoxDisableBlock->setCheckState(Qt::Checked);
		}

		if (pInspectionWindow->m_CheckOnlyHorizontalLines)
		{
			ui.checkBoxUseOnlyLineCheck->setCheckState(Qt::Checked);
		}
		else
		{
			ui.checkBoxUseOnlyLineCheck->setCheckState(Qt::Unchecked);
		}

		if (!pInspectionWindow->m_EnableInspection)
		{
			ShowLineCheckParameter(false);
			ui.frameDashBoardPrintProperty->hide();
			ui.frameDashBoardPrintTolerance->hide();
			ui.checkBoxUseOnlyLineCheck->hide();
			ui.labelOnlyLineCheck->hide();
		}
		else
		{
			ui.checkBoxUseOnlyLineCheck->show();
			ui.labelOnlyLineCheck->show();
			if (pInspectionWindow->m_CheckOnlyHorizontalLines)
			{
				ShowLineCheckParameter(true);
				ui.frameDashBoardPrintProperty->hide();
				ui.frameDashBoardPrintTolerance->hide();
				//ui.checkBoxUseOnlyLineCheck->show();
				//ui.labelOnlyLineCheck->show();
			}
			else
			{
				ShowLineCheckParameter(false);
				ui.frameDashBoardPrintProperty->show();
				ui.frameDashBoardPrintTolerance->show();
			}
		}

		if (RefSubImageFile.exists())
		{
			QImage RefSubImage;
			RefSubImage.load(ReferenceLocation);


			int ScaledWidth  = RefSubImage.width();
			int ScaledHeight = RefSubImage.height();
			if (GetImageData())
				GetImageData()->GetDisplayZoomedSizeFormatImage(ScaledWidth, ScaledHeight);

			RefSubImage = RefSubImage.scaled(ScaledWidth, ScaledHeight);
			ui.labelReferenceImage->setPixmap(QPixmap::fromImage(RefSubImage));
			ui.labelReferenceImage->show();
		}
		else
			ui.labelReferenceImage->hide();
    }
	m_SetupWindow = true;
}


void InspectionRectDialog::SlotOKButtonIsClicked()
{
	InspectionWindow *pInspectionWindow = GetImageData()->GetInspectionWindowByIndex(m_CurrentInspectionRectIndex);

	if (pInspectionWindow)
	{
		if (!m_DeleteButtonOn)
		{
			pInspectionWindow->m_ModelName = ui.lineEditInspectionWindowName->text();
			if (ui.checkBoxUseOnlyLineCheck->checkState() == Qt::Checked)
			{
				double PixelSize=GetImageData()->GetPixelSize();
				pInspectionWindow->m_CheckOnlyHorizontalLines = true;
				if (PixelSize > 0.0)
				{
					pInspectionWindow->m_ModelHeightReference = ui.doubleSpinBoxBlockHeight->value() / PixelSize;
					pInspectionWindow->m_ModelWidthReference  = ui.doubleSpinBoxBlockWidth->value() / PixelSize;
				}
			}
			else
				pInspectionWindow->m_CheckOnlyHorizontalLines = false;
			pInspectionWindow->m_NumberHorizontalLines = ui.doubleSpinBoxNumberLines->value();

			if (ui.radioButtonPrintPropertyDynamic->isChecked())
				pInspectionWindow->m_MeasureVarianteDiffImage = false;
			else
				pInspectionWindow->m_MeasureVarianteDiffImage = true;

			if (ui.checkBoxDisableBlock->checkState() == Qt::Checked)
				pInspectionWindow->m_EnableInspection = false;
			else
				pInspectionWindow->m_EnableInspection = true;

			pInspectionWindow->m_PrintErrorTolInPercent = ui.doubleSpinBoxPrintErrorTolInPercent->value();

			GetImageData()->WriteProductData();
		}
	}
}


void InspectionRectDialog::SlotDeleteCliced()
{

}


PopupDialogInspectionRectDialog::PopupDialogInspectionRectDialog(ImageData *pImageData, QWidget *parent) : PopupDialog(parent)
{
	m_Dialog = new InspectionRectDialog(pImageData, this);
	QBoxLayout *box = new QVBoxLayout();
	centralWidget()->setLayout(box);

	setWindowTitle(tr("Messfenster"));

	box->addWidget(m_Dialog);
	m_button_box = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
	
	connect(m_button_box, &QDialogButtonBox::clicked, [=](QAbstractButton *button)
	{
		switch (m_button_box->standardButton(button))
		{
		case QDialogButtonBox::Apply:
			m_Dialog->SlotOKButtonIsClicked();
			this->accept();// close();
			break;
		case QDialogButtonBox::Cancel:
			this->reject();// close();
			break;
		default:
			break;
		}
	});
	box->addWidget(m_button_box);
}


void PopupDialogInspectionRectDialog::SetInspectionRectID(int iD, bool DeleteButtonOn,const QString &CancelText,const QString &ApplyText)
{
	if (m_Dialog)
	{
		m_button_box->button(QDialogButtonBox::Cancel)->setText(CancelText);
		m_button_box->button(QDialogButtonBox::Apply)->setText(ApplyText);
		m_Dialog->SetInspectionRectID(iD, DeleteButtonOn);
	}
}
