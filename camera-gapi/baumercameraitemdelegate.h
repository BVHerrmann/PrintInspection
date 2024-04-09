#ifndef BAUMERCAMERAITEMDELEGATE_H
#define BAUMERCAMERAITEMDELEGATE_H

#include <QStyledItemDelegate>

class BaumerCameraItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaumerCameraItemDelegate(QObject *parent = 0);

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

signals:

public slots:
    void valueChanged();
    void valueChanged(const QString &text);

};

#endif // BAUMERCAMERAITEMDELEGATE_H
