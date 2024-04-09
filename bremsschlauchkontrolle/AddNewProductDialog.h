#pragma once

#include <QDialog>
#include "ui_AddNewProductDialog.h"

class ProductDialog;
class AddNewProductDialog : public QDialog
{
	Q_OBJECT

public:
	AddNewProductDialog(ProductDialog *parent);
	~AddNewProductDialog();
	ProductDialog *GetProductDialog() { return m_ProductDialog; }
	void showEvent(QShowEvent *);
	void SlotMoveWidget();
	QString GetInputString();
	QString GetSelectedProduct();
	void ClearCombobox();
	void AddItemToCombobox(QString &item);
	void OpenDialogNewInput(QString &Message);

public slots:
	void SlotOpenCombobox();
	void SlotYes();
	void SlotNo();

private:
	Ui::AddNewProductDialog ui;
	ProductDialog           *m_ProductDialog;
};
