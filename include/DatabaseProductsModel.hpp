#pragma once

#include <QStyledItemDelegate>
#include <QSqlTableModel>
#include <QLineEdit>
#include <QMessageBox>
#include "CodeEan.hpp"



class DatabaseProductsModel : public QSqlTableModel
{
    Q_OBJECT

protected:
    QString selectStatement() const override
    {
        return "SELECT id, name, codeEan, quantity, unitCode FROM " + this->tableName();
    }

public:
    DatabaseProductsModel(QObject* parent = nullptr, const QSqlDatabase& db = QSqlDatabase()) :
        QSqlTableModel(parent, db)
    {
        this->setTable("products");
        this->setEditStrategy(QSqlTableModel::OnFieldChange);
        
        this->setHeaderData(0, Qt::Horizontal, "Id");
        this->setHeaderData(1, Qt::Horizontal, "Nazwa");
        this->setHeaderData(2, Qt::Horizontal, "Kod Kreskowy");
        this->setHeaderData(3, Qt::Horizontal, "Ilość");
        this->setHeaderData(4, Qt::Horizontal, "Jednostka");
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
