#include "WidgetEditProductData.h"
#include "MainAppPrintCheck.h"
#include "ProductData.h"
#include <bmessagebox.h>
#include "WidgetEditMeasureParameter.h"
#include "ImageData.h"


WidgetEditProductData::WidgetEditProductData(MainAppPrintCheck *pParent) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pParent;
	connect(ui.pushButtonApplySettings, &QPushButton::clicked, this, &WidgetEditProductData::SlotApplySettings);
}


WidgetEditProductData::~WidgetEditProductData()
{
}


void WidgetEditProductData::showEvent(QShowEvent *event)
{
	if (GetMainAppPrintCheck())
	{
		ProductData *pCurrentProduct = GetMainAppPrintCheck()->GetCurrentProductData();
		if (pCurrentProduct)
		{
			ui.doubleSpinBoxProductID->setValue(pCurrentProduct->m_ProductID);
			ui.doubleSpinBoxProductDiameter->setValue(pCurrentProduct->m_ProductDiameter);
			//ui.doubleSpinBoxPrintErrorTolInPercent->setValue(pCurrentProduct->m_PrintErrorTolInPercent);
			ui.doubleSpinBoxPositionTolInMM->setValue(pCurrentProduct->m_PositionTolInMM);
			ui.doubleSpinBoxFormatLenghtTolInMM->setValue(pCurrentProduct->m_FormatLenghtTolInMM);
			ui.doubleSpinBoxBlockHeightTolInMM->setValue(pCurrentProduct->m_BlockHeightTolInMM);
			ui.doubleSpinBoxBlockWidthTolInMM->setValue(pCurrentProduct->m_BlockWidthTolInMM);
			ui.lineEditCameraTopName->setText(pCurrentProduct->m_CameraTopType);
			ui.lineEditCameraBotName->setText(pCurrentProduct->m_CameraBotType);
    		ui.doubleSpinBoxFormatLenghtTopCamera->setValue(pCurrentProduct->m_FormatLenghtInMMTopCamera);
			ui.doubleSpinBoxFormatLenghtBotCamera->setValue(pCurrentProduct->m_FormatLenghtInMMBotCamera);
		}
	}
}


void WidgetEditProductData::SlotApplySettings()
{
	bool writeProductData=true;
	QString ErrorMsg;

	if (GetMainAppPrintCheck())
	{
		ImageData  *pImageData=NULL;
		ProductData *pCurrentProduct = GetMainAppPrintCheck()->GetCurrentProductData();
		if (pCurrentProduct)
		{
			if (pCurrentProduct->m_ProductID != ui.doubleSpinBoxProductID->value())
			{
				if (GetMainAppPrintCheck()->ExistProductID(ui.doubleSpinBoxProductID->value()))
				{
					int NewID=GetMainAppPrintCheck()->GenerateNewProductID();
					BMessageBox::critical(NULL, tr("Invalid ProductID"), tr("The Next Valid ID:%1!").arg(NewID));
					writeProductData = false;
				}
			}
			if (writeProductData)
			{
				pCurrentProduct->m_ProductID                      = ui.doubleSpinBoxProductID->value();
				pCurrentProduct->m_ProductDiameter                = ui.doubleSpinBoxProductDiameter->value();
				//pCurrentProduct->m_PrintErrorTolInPercent         = ui.doubleSpinBoxPrintErrorTolInPercent->value();
				pCurrentProduct->m_PositionTolInMM                = ui.doubleSpinBoxPositionTolInMM->value();
				pCurrentProduct->m_FormatLenghtTolInMM            = ui.doubleSpinBoxFormatLenghtTolInMM->value();
				pCurrentProduct->m_BlockHeightTolInMM             = ui.doubleSpinBoxBlockHeightTolInMM->value();
				pCurrentProduct->m_BlockWidthTolInMM              = ui.doubleSpinBoxBlockWidthTolInMM->value();
				pCurrentProduct->m_CameraTopType                  = ui.lineEditCameraTopName->text();
				pCurrentProduct->m_CameraBotType                  = ui.lineEditCameraBotName->text();
				pCurrentProduct->m_FormatLenghtInMMTopCamera      = ui.doubleSpinBoxFormatLenghtTopCamera->value();
				pCurrentProduct->m_FormatLenghtInMMBotCamera      = ui.doubleSpinBoxFormatLenghtBotCamera->value();
				pCurrentProduct->WriteProductData(ErrorMsg);
			}
		}
	}
}
