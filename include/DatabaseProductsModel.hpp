#pragma once

#include <QStyledItemDelegate>
#include <QSqlTableModel>
#include <QLineEdit>
#include <QMessageBox>
#include <qsqldatabase.h>
#include "CodeEan.hpp"



class DatabaseProductsModel : public QSqlTableModel
{
    Q_OBJECT

public:
    using QSqlTableModel::QSqlTableModel;
    DatabaseProductsModel(QObject* parent = nullptr, const QSqlDatabase& db = QSqlDatabase()) :
        QSqlTableModel(parent, db)
    {
    }

    class EanDelegate : public QStyledItemDelegate
    {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
        {
            auto *ed = qobject_cast<QLineEdit*>(editor);
            if (!ed) return;

            const QString text = ed->text();
            try
            {
                CodeEan ean(text);
                model->setData(index, text);
            }
            catch (const std::exception& ex)
            {
                QMessageBox::warning(editor, "Błąd kodu EAN", ex.what());
                const_cast<QLineEdit*>(ed)->setFocus();
            }
        }
    };
};
