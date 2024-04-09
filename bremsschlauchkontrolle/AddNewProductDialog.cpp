#include "AddNewProductDialog.h"

#include "ProductDialog.h"
#include "qtimer.h"
#include "GlobalConst.h"


AddNewProductDialog::AddNewProductDialog(ProductDialog *pProductDialog)	: QDialog(pProductDialog)
{
	ui.setupUi(this);
	m_ProductDialog = pProductDialog;

	connect(ui.pushButtonOK,                   &QPushButton::clicked, this, &AddNewProductDialog::SlotYes);
	connect(ui.pushButtonCancel,               &QPushButton::clicked, this, &AddNewProductDialog::SlotNo);
	connect(ui.toolButtonSelectInheritProduct, &QToolButton::clicked, this, &AddNewProductDialog::SlotOpenCombobox);
}


AddNewProductDialog::~AddNewProductDialog()
{
}


void AddNewProductDialog::SlotOpenCombobox()
{
	ui.comboBox->showPopup(); 
}


void AddNewProductDialog::ClearCombobox()
{
	ui.comboBox->clear();
}


QString AddNewProductDialog::GetSelectedProduct()
{
	return ui.comboBox->currentText();
}


void AddNewProductDialog::AddItemToCombobox(QString &item)
{
	ui.comboBox->addItem(item);
}


void AddNewProductDialog::showEvent(QShowEvent *)
{
	ui.lineEditInputData->setFocus();
	activateWindow();
	QTimer::singleShot(10, this, &AddNewProductDialog::SlotMoveWidget);
}


void AddNewProductDialog::SlotMoveWidget()
{
	move(geometry().x(), STANDARD_Y_DIALOG_POSITION);
}


QString AddNewProductDialog::GetInputString()
{
	return ui.lineEditInputData->text();
}


void AddNewProductDialog::SlotYes()
{
	if (GetProductDialog())
	{
		QString NewProductName      = GetInputString();
		QString CopyFromProductName = GetSelectedProduct();
		GetProductDialog()->InsertNewProduct(NewProductName, CopyFromProductName);
		GetProductDialog()->UpdateProductList();
		accept();
	}
}


void AddNewProductDialog::SlotNo()
{
	accept();
}


void AddNewProductDialog::OpenDialogNewInput(QString &Message)
{
	QRegExp rx("^\\w+$");
	QValidator *validator = new QRegExpValidator(rx, this);

	ui.lineEditInputData->setReadOnly(false);
	ui.lineEditInputData->show();
	ui.lineEditInputData->setValidator(validator);
	ui.labelMasseage->setText(Message);
	ui.lineEditInputData->setFocus();
	show();
}
