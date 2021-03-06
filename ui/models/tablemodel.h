#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include "lt/rom/table.h"
#include <QAbstractTableModel>

class TableModel : public QAbstractTableModel
{
public:
    TableModel() = default;

    void setTable(lt::Table * table) noexcept;
    inline lt::Table * table() const noexcept { return table_; }

    virtual int rowCount(const QModelIndex & parent) const override;
    virtual int columnCount(const QModelIndex & parent) const override;
    virtual QVariant data(const QModelIndex & index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;
    virtual bool setData(const QModelIndex & index, const QVariant & value,
                         int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex & index) const override;

private:
    lt::Table * table_{nullptr};
};

#endif
