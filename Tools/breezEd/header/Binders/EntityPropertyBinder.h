#ifndef ENTITYPROPERTYBINDER_H
#define ENTITYPROPERTYBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>

#include <beEntitySystem/beEntity.h>

class QTreeView;
class QStandardItem;

/// Create entity command class.
class EntityPropertyBinder : public QObject, public lean::noncopyable
{
	Q_OBJECT

private:
	lean::resource_ptr<beEntitySystem::Entity> m_pEntity;

public:
	/// Constructor.
	EntityPropertyBinder(beEntitySystem::Entity *pEntity, QTreeView *pTree, QStandardItem *pParentItem, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~EntityPropertyBinder();

	/// Property provider.
	beEntitySystem::Entity* entity() const { return m_pEntity; }
};

#endif
