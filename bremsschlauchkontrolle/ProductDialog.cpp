#include "ProductDialog.h"
#include "MainAppPrintCheck.h"
#include "GlobalConst.h"
#include "ProductData.h"
#include "AddNewProductDialog.h"
#include "qlistwidget.h"
#include "bmessagebox.h"


ProductDialog::ProductDialog(MainAppPrintCheck *pMainAppPrintCheck, QWidget *parent) : QDialog(parent)
, m_MainAppPrintCheck(NULL)
{
	ui.setupUi(this);
	m_MainAppPrintCheck        = pMainAppPrintCheck;
	m_PopupDialogProductDialog = (PopupDialogProductDialog*)(parent);
	
	connect(ui.pushButtonClose,                 &QPushButton::clicked, this, &ProductDialog::SlotCloseProductDialog);
	connect(ui.pushButtonReadAndAppendProduct,  &QPushButton::clicked, this, &ProductDialog::SlotReadAndAppendProduct);
	connect(ui.pushButtonLoadProduct,           &QPushButton::clicked, this, &ProductDialog::SlotLoadProduct);
	connect(ui.pushButtonRenameProduct,         &QPushButton::clicked, this, &ProductDialog::SlotRenameProduct);
	connect(ui.pushButtonLoadDeleteProduct,     &QPushButton::clicked, this, &ProductDialog::SlotDeleteProduct);
	connect(ui.pushButtonEditProduct,           &QPushButton::clicked, this, &ProductDialog::SlotEditProduct);
	connect(ui.pushButtonExportProduct,         &QPushButton::clicked, this, &ProductDialog::SlotExportProduct);
	connect(ui.pushButtonImportProduct,         &QPushButton::clicked, this, &ProductDialog::SlotImportProduct);

	ui.pushButtonEditProduct->hide();

	m_AddNewProductDialog = new AddNewProductDialog(this);
	m_AddNewProductDialog->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
}


void ProductDialog::showEvent(QShowEvent *)
{
	UpdateProductList();
	activateWindow();
}


void ProductDialog::UpdateProductList()
{
	if (GetMainAppPrintCheck())
	{
		QString LastProductName = GetMainAppPrintCheck()->GetCurrentProductName();
		QString ProductName;
		QList <ProductData*> *pProductList = GetMainAppPrintCheck()->GetListProducts();
		bool LastProductNameIsIn = false;
		int row = 1;

		ui.listProducts->clear();
		if (pProductList->count() > 0)
		{
			ui.listProducts->insertItem(0, GetMainAppPrintCheck()->GetCurrentProductName());
			for (int i = 0; i < pProductList->count(); i++)
			{
				ProductName = pProductList->at(i)->GetProductName();
				if(GetMainAppPrintCheck()->GetCurrentProductName() != ProductName)
				    ui.listProducts->insertItem(row, ProductName);
				if (ProductName == LastProductName)
					LastProductNameIsIn = true;
				row++;
			}
			if (!LastProductNameIsIn)
			{
				LastProductName = pProductList->at(0)->GetProductName();
				GetMainAppPrintCheck()->ShowAndSetCurrentProductName(LastProductName);
			}
			ui.lineEditCurrentProduct->setText(GetMainAppPrintCheck()->GetCurrentProductName());
			ui.listProducts->setCurrentRow(0, QItemSelectionModel::Select);
			ui.listProducts->setFocus();
		}
		else
			ui.lineEditCurrentProduct->setText("");
	}
}


ProductDialog::~ProductDialog()
{
}


void ProductDialog::SlotExportProduct()
{
	QList<QListWidgetItem *> m_List = ui.listProducts->selectedItems();
	QString SourcePath, SelectetProductName, CurrentZipFileName,ZipLocation;

	if (m_List.count() > 0)
	{
		SelectetProductName = m_List.at(0)->text();
		ZipLocation         = QFileDialog::getExistingDirectory(this, tr("Select Location Save Product Data"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    	CurrentZipFileName  = ZipLocation + SelectetProductName + QString(".dat");
		SourcePath          = GetMainAppPrintCheck()->GetProductLocation() + QString("//") + SelectetProductName;
		CompressFolder(SourcePath, CurrentZipFileName);
	}
}


void ProductDialog::SlotImportProduct()
{
	QString ErrorMsg,fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Product (*.dat)"));

	if (!fileName.isEmpty() && GetMainAppPrintCheck())
	{
		QFileInfo FileInfo(fileName);
		QList<QListWidgetItem *> m_List = ui.listProducts->selectedItems();
		QString SelectetProductName, ProductName,BaseName = FileInfo.baseName();
		QList <ProductData*> *pProductList = GetMainAppPrintCheck()->GetListProducts();
		bool ImportProduct = true;
		bool ImportProductExist = false;

		if(m_List.count() > 0)
			SelectetProductName = m_List.at(0)->text();
		
		for (int i = 0; i < pProductList->count(); i++)
		{
			if (pProductList->at(i)->GetProductName() == BaseName)
			{
				ImportProductExist = true;
				if (BMessageBox::information(NULL, tr("Overwrite Product"), tr("Product %1 Exist, Override?").arg(BaseName), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				{
					ImportProduct = false;
				}
				break;
			}
		}
		if (ImportProduct)
		{
			QString ProductLocation = GetMainAppPrintCheck()->GetProductLocation() + QString("/") + BaseName;
			DeCompressFolder(fileName, ProductLocation);
			if (GetMainAppPrintCheck())
				GetMainAppPrintCheck()->LoadAllProductFiles(ErrorMsg);
			UpdateProductList();
		}
	}
}


void ProductDialog::CompressFolder(QString &SourceFolder, QString &DestinationFile)
{
	m_DestinationFileCompressFolder.setFileName(DestinationFile);
	if (m_DestinationFileCompressFolder.open(QIODevice::WriteOnly))
	{
		m_DataStreamCompressFolder.setDevice(&m_DestinationFileCompressFolder);
		Compress(SourceFolder,QString(""));
		m_DestinationFileCompressFolder.close();
	}
}

void ProductDialog::Compress(QString &SourceFolder,QString &prefex)
{
	QDir dir(SourceFolder);
	if (!dir.exists())
	{
		return;
	}

	dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	QFileInfoList folderlist = dir.entryInfoList();
	for (int i = 0; i < folderlist.length(); i++)
	{
		QString folderName = folderlist.at(i).fileName();
		QString folderPah  = dir.absolutePath() + "/" + folderName;
		QString newPrefix  = prefex + "/" + folderName;
		Compress(folderPah, newPrefix);
	}

	dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
	QFileInfoList filelist = dir.entryInfoList();
	for (int i = 0; i < filelist.length(); i++)
	{
		QString FileName = dir.absolutePath() + "/" + filelist.at(i).fileName();
		QFile file(dir.absolutePath() + "/" + filelist.at(i).fileName());
		if (file.open(QIODevice::ReadOnly))
		{
			m_DataStreamCompressFolder << QString(prefex + "/" + filelist.at(i).fileName());
			m_DataStreamCompressFolder << qCompress(file.readAll());
			file.close();
		}
	}
}


void ProductDialog::DeCompressFolder(QString &SourceFolder, QString &DestinationFolder)
{
	QFile src(SourceFolder);

	QDir().mkpath(DestinationFolder);
	if (src.open(QIODevice::ReadOnly))
	{
		QDataStream dataStream;

		dataStream.setDevice(&src);

		while (!dataStream.atEnd())
		{
			QString Subfolder,fileName;
			QByteArray data;

			dataStream >> fileName >> data;
			for (int i = fileName.length() - 1; i > 0; i--)
			{
				if ((QString(fileName.at(i)) == QString("//")) || (QString(fileName.at(i)) == QString("/")))
				{
					Subfolder = fileName.left(i);
					QDir().mkpath(DestinationFolder + "/" + Subfolder);
					break;
				}
			}

			QFile outFile(DestinationFolder + "/" + fileName);
			if (outFile.open(QIODevice::WriteOnly))
			{
				outFile.write(qUncompress(data));
				outFile.close();
			}
		}
		src.close();
	}
}


void ProductDialog::SlotReadAndAppendProduct()
{
	bool rv = false;
	int retVal = ERROR_CODE_NO_ERROR;
	
	
	if (GetAddNewProductDialog())
	{
		GetAddNewProductDialog()->ClearCombobox();
		GetAddNewProductDialog()->AddItemToCombobox(QString(""));
		for (int i = 0; i < ui.listProducts->count(); i++)
		{
			QString value = ui.listProducts->item(i)->text();
			GetAddNewProductDialog()->AddItemToCombobox(value);
		}
		GetAddNewProductDialog()->OpenDialogNewInput(tr("Insert New Product Name"));//show dialog
	}
}


void ProductDialog::InsertNewProduct(QString &NewProductName, QString &CopyFromProductName)
{
	QString  LineEdit, ErrorMsg;
	int retVal = ERROR_CODE_NO_ERROR;
	
	if (GetMainAppPrintCheck())
	{
		if (!NewProductName.isEmpty())
		{
			if (GetMainAppPrintCheck()->ExistProduct(NewProductName))
			{
				if (BMessageBox::information(NULL, tr("Overwrite"), tr("Product %1 Exist, Override?").arg(NewProductName), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
				{
					GetMainAppPrintCheck()->RemoveProduct(NewProductName);
					retVal = GetMainAppPrintCheck()->WriteAndInsertNewProduct(NewProductName, CopyFromProductName, ErrorMsg);
					if (retVal == ERROR_CODE_NO_ERROR)
					{
						GetMainAppPrintCheck()->ActivateProduct(NewProductName);
						
					}
					else
					{
						GetMainAppPrintCheck()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
					}
				}
			}
			else
			{
				retVal = GetMainAppPrintCheck()->WriteAndInsertNewProduct(NewProductName, CopyFromProductName, ErrorMsg);
				if (retVal == ERROR_CODE_NO_ERROR)
				{
				    GetMainAppPrintCheck()->ActivateProduct(NewProductName);
					
				}
				else
				{
					GetMainAppPrintCheck()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
				}
			}
		}
	}
}


void ProductDialog::SlotLoadProduct()
{
	if (GetMainAppPrintCheck())
	{
		QList<QListWidgetItem *> m_List = ui.listProducts->selectedItems();
		QString  NewProductName;

		if (m_List.count() > 0)
		{
			NewProductName = m_List.at(0)->text();
			ui.lineEditCurrentProduct->setText(NewProductName);
		}
		GetMainAppPrintCheck()->ActivateProduct(NewProductName);
	}
}


void ProductDialog::SlotRenameProduct()
{
	QList<QListWidgetItem *> m_List = ui.listProducts->selectedItems();
	QString ErrorMsg, RenameName;
	int rv;

	if (m_List.count() > 0)
	{
		RenameName = m_List.at(0)->text();
		bool ok;

		if (GetMainAppPrintCheck()->GetCurrentProductName() == RenameName)
		{
			BMessageBox::information(NULL, tr("Rename Not Possible"), tr("Can Not Rename Active Product %1. ").arg(RenameName), QMessageBox::Ok);
		}
		else
		{
			QString NewName = QInputDialog::getText(this, tr("Insert New Name"), tr("Rename Product: %1?").arg(RenameName), QLineEdit::Normal, QString("TypeXY"), &ok, Qt::Tool | Qt::FramelessWindowHint);
			if (ok)
			{
				rv=GetMainAppPrintCheck()->RenameAndActivateProduct(RenameName, NewName, ErrorMsg);
				if (rv != ERROR_CODE_NO_ERROR)
				{
					BMessageBox::warning(NULL, tr("Can Not Rename"), tr("Can Not Rename Product Exist %1. ").arg(RenameName), QMessageBox::Ok);
				}
			}
		}
	}
}


void ProductDialog::SlotDeleteProduct()
{
	QList<QListWidgetItem *> m_List = ui.listProducts->selectedItems();
	
	if (m_List.count() > 0)
	{
		if (GetMainAppPrintCheck())
		{
			QString ProductName = m_List.at(0)->text();
			if (GetMainAppPrintCheck()->GetCurrentProductName() == ProductName)
			{
				BMessageBox::information(NULL, tr("Delete Not Possible"), tr("Can Not Delete Active Product %1. ").arg(ProductName), QMessageBox::Ok);
			}
			else
			{
				if (BMessageBox::information(NULL, tr("Delete"), tr("Delete Product %1?").arg(ProductName), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
				{
					QString ErrorMsg, PathAndFileName = GetMainAppPrintCheck()->GetProductLocation() + QString("/") + ProductName;// +QString(".dat");
    				int rv = ERROR_CODE_NO_ERROR;


					GetMainAppPrintCheck()->RemoveProduct(ProductName);
					rv = GetMainAppPrintCheck()->LoadAllProductFiles(ErrorMsg);
					if (rv != ERROR_CODE_NO_ERROR)
					{
						GetMainAppPrintCheck()->SlotAddNewMessage(ErrorMsg, QtMsgType::QtFatalMsg);
					}
					UpdateProductList();
					GetMainAppPrintCheck()->ActivateProduct(GetMainAppPrintCheck()->GetCurrentProductName());
				}
			}
		}
	}
}

//not active
void ProductDialog::SlotEditProduct()
{
	
}


void ProductDialog::SlotCloseProductDialog()
{
	if(m_PopupDialogProductDialog)
	   m_PopupDialogProductDialog->accept();
}
