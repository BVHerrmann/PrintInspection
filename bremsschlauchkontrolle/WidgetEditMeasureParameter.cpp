#include "WidgetEditMeasureParameter.h"
#include "MainAppPrintCheck.h"
#include "ImageData.h"
#include "MeasureTaskDetectHose.h"
#include "MeasureTaskPrintCheck.h"
#include "HoseDetector.h"
#include "MatchingShapeBased.h"
#include "ModelParameter.h"
#include "PrintLineDetector.h"
#include "ProductData.h"


WidgetEditMeasureParameter::WidgetEditMeasureParameter(MainAppPrintCheck *pParent,int CameraIndex) : QWidget(NULL)
, m_MainAppPrintCheck(NULL)
, m_CameraIndex(0)
{
	ui.setupUi(this);
	m_MainAppPrintCheck = pParent;
	m_CameraIndex       = CameraIndex;
	connect(ui.pushButtonApplyMeasuringSettings, &QPushButton::clicked, this, &WidgetEditMeasureParameter::SlotApplyParameter);
	ui.frameDashBoardParameterLineCheckBinThreshold->hide();
}


WidgetEditMeasureParameter::~WidgetEditMeasureParameter()
{
}


void WidgetEditMeasureParameter::showEvent(QShowEvent *event)
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
			    
			    if (pImageData->GetMeasureTaskDetectHose())
			    {
					HoseDetector *pHoseDetector = pImageData->GetMeasureTaskDetectHose()->GetHoseDetector();
					if (pHoseDetector)
					{
						ModelParameter *pModelParameter = pHoseDetector->GetModelParameter();
						if (pModelParameter)
						{
							ui.doubleSpinBoxModelAccsepanceThresholdHoseDetection->setValue(pModelParameter->m_ModelAcceptanceThresholdInPercent*100.0);
							ui.doubleSpinBoxModelMaxScaleFactorHoseDetection->setValue(pModelParameter->m_MaxModelScaleFactor);
							ui.doubleSpinBoxModelMinScaleFactorHoseDetection->setValue(pModelParameter->m_MinModelScaleFactor);
							ui.doubleSpinBoxModelSearchAngleNegHoseDetection->setValue(pModelParameter->m_SearchAngleRangeNegInDegree);
							ui.doubleSpinBoxModelSearchAnglePosHoseDetection->setValue(pModelParameter->m_SearchAngleRangePosInDegree);
							ui.doubleSpinBoxMinScoreHoseDetection->setValue(pModelParameter->m_MinScore*100.0);
						}
					}
			    }
				MatchingShapeBased *pMatchingShapeBased = pImageData->GetMatchingShapeBased();
				if (pMatchingShapeBased)
				{
					ModelParameter *pModelParameter = pMatchingShapeBased->GetModelParameter();
					if (pModelParameter)
					{
						ui.doubleSpinBoxModelAccsepanceThresholdPrintCheck->setValue(pModelParameter->m_ModelAcceptanceThresholdInPercent*100.0);
						ui.doubleSpinBoxModelMaxScaleFactorPrintCheck->setValue(pModelParameter->m_MaxModelScaleFactor);
						ui.doubleSpinBoxModelMinScaleFactorPrintCheck->setValue(pModelParameter->m_MinModelScaleFactor);
						ui.doubleSpinBoxModelSearchAngleNegPrintCheck->setValue(pModelParameter->m_SearchAngleRangeNegInDegree);
						ui.doubleSpinBoxModelSearchAnglePosPrintCheck->setValue(pModelParameter->m_SearchAngleRangePosInDegree);
						ui.doubleSpinBoxMinScorePrintCheck->setValue(pModelParameter->m_MinScore*100.0);
						ui.doubleSpinBoxGrayScaleThresholdPrintCheck->setValue(pModelParameter->m_GrayThreshold);
					}
				}
				MatchingShapeBased *pMatchingFormatWindow = pImageData->GetMatchingFormatWindow();//Parameter für die Messung des Formates
				if (pMatchingFormatWindow)
				{
					ModelParameter *pModelParameter = pMatchingFormatWindow->GetModelParameter();
					if (pModelParameter)
					{
						ui.doubleSpinBoxModelAccsepanceThresholdFormatDetection->setValue(pModelParameter->m_ModelAcceptanceThresholdInPercent*100.0);
						ui.doubleSpinBoxModelMaxScaleFactorFormatDetection->setValue(pModelParameter->m_MaxModelScaleFactor);
						ui.doubleSpinBoxModelMinScaleFactorFormatDetection->setValue(pModelParameter->m_MinModelScaleFactor);
						ui.doubleSpinBoxModelSearchAngleNegFormatDetection->setValue(pModelParameter->m_SearchAngleRangeNegInDegree);
						ui.doubleSpinBoxModelSearchAnglePosFormatDetection->setValue(pModelParameter->m_SearchAngleRangePosInDegree);
						ui.doubleSpinBoxMinScoreFormatDetection->setValue(pModelParameter->m_MinScore*100.0);
					}
					
				}
				PrintLineDetector *pPrintLineDetector = pImageData->GetPrintLineDetector();
				if (pPrintLineDetector)
				{
					ModelParameter *pModelParameter = pPrintLineDetector->GetModelParameter();
					if (pModelParameter)
					{
						ui.doubleSpinBoxGrayScaleThresholdLineCheck->setValue(pModelParameter->m_GrayThreshold);
						ui.doubleSpinBoxThresholdToleranceFactorLineCheck->setValue(pModelParameter->m_ThresholToleranceFactor);
						ui.doubleSpinBoxMaxScaleFactorXLineCheck->setValue(pModelParameter->m_MaxModelScaleFactor);
						ui.doubleSpinBoxMinScaleFactorXLineCheck->setValue(pModelParameter->m_MinModelScaleFactor);
						ui.doubleSpinBoxMaxScaleFactorYLineCheck->setValue(pModelParameter->m_MaxModelScaleFactorY);
						ui.doubleSpinBoxMinScaleFactorYLineCheck->setValue(pModelParameter->m_MinModelScaleFactorY);
						ui.doubleSpinBoxModelSearchAngleNegLineCheck->setValue(pModelParameter->m_SearchAngleRangeNegInDegree);
						ui.doubleSpinBoxModelSearchAnglePosLineCheck->setValue(pModelParameter->m_SearchAngleRangePosInDegree);
						ui.doubleSpinBoxModelAccsepanceThresholdLineCheck->setValue(pModelParameter->m_ModelAcceptanceThresholdInPercent*100.0);
						ui.doubleSpinBoxMinScoreLineCheck->setValue(pModelParameter->m_MinScore*100.0);
					}
				}
		}
	}
}


void WidgetEditMeasureParameter::SlotApplyParameter()
{
	if (GetMainAppPrintCheck())
	{
		ImageData *pImageData = GetMainAppPrintCheck()->GetImageData(m_CameraIndex);
		if (pImageData)
		{
			    if (pImageData->GetMeasureTaskDetectHose())
			    {
					HoseDetector *pHoseDetector = pImageData->GetMeasureTaskDetectHose()->GetHoseDetector();
					if (pHoseDetector)
					{
						ModelParameter *pModelParameter = pHoseDetector->GetModelParameter();
						if (pModelParameter)
						{
							pModelParameter->m_ModelAcceptanceThresholdInPercent = ui.doubleSpinBoxModelAccsepanceThresholdHoseDetection->value() / 100.0;
							pModelParameter->m_MaxModelScaleFactor = ui.doubleSpinBoxModelMaxScaleFactorHoseDetection->value();
							pModelParameter->m_MinModelScaleFactor = ui.doubleSpinBoxModelMinScaleFactorHoseDetection->value();
							pModelParameter->m_SearchAngleRangeNegInDegree = ui.doubleSpinBoxModelSearchAngleNegHoseDetection->value();
							pModelParameter->m_SearchAngleRangePosInDegree = ui.doubleSpinBoxModelSearchAnglePosHoseDetection->value();
							pModelParameter->m_MinScore = ui.doubleSpinBoxMinScoreHoseDetection->value() / 100.0;
						}
						pHoseDetector->SaveSettings();
					}
			    }
				MatchingShapeBased *pMatchingShapeBased = pImageData->GetMatchingShapeBased();//Parameter für die Messung der einzelnen Blöcke im Format
				if (pMatchingShapeBased)
				{
					ModelParameter *pModelParameter = pMatchingShapeBased->GetModelParameter();
					if (pModelParameter)
					{
						pModelParameter->m_ModelAcceptanceThresholdInPercent      = ui.doubleSpinBoxModelAccsepanceThresholdPrintCheck->value()/100.0;
						pModelParameter->m_MaxModelScaleFactor                    = ui.doubleSpinBoxModelMaxScaleFactorPrintCheck->value();
						pModelParameter->m_MinModelScaleFactor                    = ui.doubleSpinBoxModelMinScaleFactorPrintCheck->value();
						pModelParameter->m_SearchAngleRangeNegInDegree            = ui.doubleSpinBoxModelSearchAngleNegPrintCheck->value();
						pModelParameter->m_SearchAngleRangePosInDegree            = ui.doubleSpinBoxModelSearchAnglePosPrintCheck->value();
						pModelParameter->m_MinScore                               = ui.doubleSpinBoxMinScorePrintCheck->value() / 100.0;
						pModelParameter->m_GrayThreshold                          = ui.doubleSpinBoxGrayScaleThresholdPrintCheck->value();
					}
					pMatchingShapeBased->SaveSettings();
				}
				MatchingShapeBased *pMatchingFormatWindow = pImageData->GetMatchingFormatWindow();//Parameter für die Messung des Formates
				if (pMatchingFormatWindow)
				{
					ModelParameter *pModelParameter = pMatchingFormatWindow->GetModelParameter();
					if (pModelParameter)
					{
						pModelParameter->m_ModelAcceptanceThresholdInPercent  = ui.doubleSpinBoxModelAccsepanceThresholdFormatDetection->value() / 100.0;
						pModelParameter->m_MaxModelScaleFactor                = ui.doubleSpinBoxModelMaxScaleFactorFormatDetection->value();
						pModelParameter->m_MinModelScaleFactor                = ui.doubleSpinBoxModelMinScaleFactorFormatDetection->value();
						pModelParameter->m_SearchAngleRangeNegInDegree        = ui.doubleSpinBoxModelSearchAngleNegFormatDetection->value();
						pModelParameter->m_SearchAngleRangePosInDegree        = ui.doubleSpinBoxModelSearchAnglePosFormatDetection->value();
						pModelParameter->m_MinScore                           = ui.doubleSpinBoxMinScoreFormatDetection->value() / 100.0;
					}
					pMatchingFormatWindow->SaveSettings();
				}
				PrintLineDetector *pPrintLineDetector = pImageData->GetPrintLineDetector();
				if (pPrintLineDetector)
				{
					ModelParameter *pModelParameter = pPrintLineDetector->GetModelParameter();
					if (pModelParameter)
					{
						pModelParameter->m_GrayThreshold                     = ui.doubleSpinBoxGrayScaleThresholdLineCheck->value();
						pModelParameter->m_ThresholToleranceFactor           = ui.doubleSpinBoxThresholdToleranceFactorLineCheck->value();
						pModelParameter->m_MaxModelScaleFactor               = ui.doubleSpinBoxMaxScaleFactorXLineCheck->value();
						pModelParameter->m_MinModelScaleFactor               = ui.doubleSpinBoxMinScaleFactorXLineCheck->value();
						pModelParameter->m_MaxModelScaleFactorY              = ui.doubleSpinBoxMaxScaleFactorYLineCheck->value();
						pModelParameter->m_MinModelScaleFactorY              = ui.doubleSpinBoxMinScaleFactorYLineCheck->value();
						pModelParameter->m_SearchAngleRangeNegInDegree       = ui.doubleSpinBoxModelSearchAngleNegLineCheck->value();
						pModelParameter->m_SearchAngleRangePosInDegree       = ui.doubleSpinBoxModelSearchAnglePosLineCheck->value();
						pModelParameter->m_ModelAcceptanceThresholdInPercent = ui.doubleSpinBoxModelAccsepanceThresholdLineCheck->value()/100.0;
						pModelParameter->m_MinScore                          = ui.doubleSpinBoxMinScoreLineCheck->value()/100.0;
						pPrintLineDetector->ClearModel();//wenn sich parameter geändert haben muss das Model neu erzeugt werden
					}
					pPrintLineDetector->SaveSettings();
				}
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
