#pragma once

#include <QWidget>
#include "qfile.h"
#include "ui_ProductDialog.h"
#include "qprocess.h"
#include "popupdialog.h"

class MainAppPrintCheck;
class InputMessageDialog;
class AddNewProductDialog;
class MainGUIPrintCheck;
class PopupDialogProductDialog;
class ProductDialog : public QDialog
{
	Q_OBJECT

public:
	ProductDialog(MainAppPrintCheck *pMainAppPrintCheck, QWidget *parent);
	~ProductDialog();
	MainAppPrintCheck *GetMainAppPrintCheck() { return m_MainAppPrintCheck; }
	InputMessageDialog    *GetInputMessageDialog() { return m_InputMessageDialog; }
	AddNewProductDialog   *GetAddNewProductDialog() { return m_AddNewProductDialog; }
	void showEvent(QShowEvent *ev);
	void UpdateProductList();
	void InsertNewProduct(QString &NewProductName, QString &CopyFromProduct);
	void Compress(QString &SourceFolder, QString &prefex);
	void CompressFolder(QString &SourceFolder, QString &DestinationFile);
	void DeCompressFolder(QString &SourceFolder, QString &DestinationFolder);

public slots:
	void SlotCloseProductDialog();
	void SlotReadAndAppendProduct();
	void SlotLoadProduct();
	void SlotRenameProduct();
	void SlotDeleteProduct();
	void SlotEditProduct();
	void SlotExportProduct();
	void SlotImportProduct();

	

private:
	Ui::ProductDialog      ui;
	MainAppPrintCheck     *m_MainAppPrintCheck;
	InputMessageDialog    *m_InputMessageDialog;
	AddNewProductDialog   *m_AddNewProductDialog;
	PopupDialogProductDialog *m_PopupDialogProductDialog;
	QDataStream m_DataStreamCompressFolder;
	QFile       m_DestinationFileCompressFolder;
};

class PopupDialogProductDialog : public PopupDialog
{

public:
	PopupDialogProductDialog(MainAppPrintCheck *pMainAppPrintCheck, MainGUIPrintCheck *pMainGUIPrintCheck) : PopupDialog((QWidget*)(pMainGUIPrintCheck))
	{
		m_ProductDialog = new ProductDialog(pMainAppPrintCheck, this);
		QBoxLayout *box = new QVBoxLayout();
		centralWidget()->setLayout(box);

		setWindowTitle(tr("Open Product"));

		box->addWidget(m_ProductDialog);
		/*QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Close);
		button_box->button(QDialogButtonBox::Apply)->setObjectName("apply");
		button_box->button(QDialogButtonBox::Cancel)->setObjectName("cancel");
		connect(button_box, &QDialogButtonBox::clicked, [=](QAbstractButton *button)
		{
			switch (button_box->standardButton(button))
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
		box->addWidget(button_box);*/

	}
	ProductDialog *GetProductDialog() { return m_ProductDialog; }

private:
	ProductDialog *m_ProductDialog;


};
