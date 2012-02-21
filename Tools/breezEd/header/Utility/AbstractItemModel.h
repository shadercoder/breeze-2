#ifndef ABSTRACTITEMMODEL_H
#define ABSTRACTITEMMODEL_H

#include "breezEd.h"

#include <QtCore/QAbstractItemModel>
#include <vector>

/// Create entity command class.
class AbstractItemModel : public QAbstractItemModel
{
	struct ItemInfo
	{
		int row;
		uint4 parent;

		ItemInfo(int row, uint4 parent)
			: row(row),
			parent(parent) { }
	};
	typedef std::vector<ItemInfo> item_info_vector;

private:
	mutable item_info_vector m_qtSucks;

protected:
	AbstractItemModel(const AbstractItemModel&) { }
	AbstractItemModel& operator =(const AbstractItemModel&) { return *this; }

public:
	/// Constructor.
	AbstractItemModel(QObject *pParent = nullptr)
		: QAbstractItemModel(pParent) { }
	/// Destructor.
	virtual ~AbstractItemModel() { }

	/// Gets a model index for the given item.
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
	{
		uint4 parentID = (parent.isValid())
			? static_cast<uint4>(parent.internalId())
			: static_cast<uint4>(-1);

		uint4 itemID = static_cast<uint4>(m_qtSucks.size());
		m_qtSucks.push_back( ItemInfo(row, parentID) );
		return createIndex(row, column, itemID);
	}
	/// Gets a parent model index for the given item.
    QModelIndex parent(const QModelIndex &child) const
	{
		if (child.isValid() && child.internalId() < m_qtSucks.size())
		{
			const ItemInfo &parentInfo = m_qtSucks[static_cast<uint4>(child.internalId())];
			return createIndex(parentInfo.row, 0, parentInfo.parent);
		}

		return QModelIndex();
	}
};

#endif
