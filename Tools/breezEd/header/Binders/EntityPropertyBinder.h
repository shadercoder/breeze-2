#ifndef ENTITYPROPERTYBINDER_H
#define ENTITYPROPERTYBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>

#include <beEntitySystem/beEntities.h>

class QTreeView;
class QStandardItem;

class SceneDocument;

/// Create entity command class.
class EntityPropertyBinder : public QObject, public lean::noncopyable
{
	Q_OBJECT

private:
	beEntitySystem::Entity *m_pEntity;

public:
	/// Constructor.
	EntityPropertyBinder(beEntitySystem::Entity *pEntity, SceneDocument *pDocument, QTreeView *pTree, QStandardItem *pParentItem, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~EntityPropertyBinder();

	/// Property provider.
	beEntitySystem::Entity* entity() const { return m_pEntity; }

public Q_SLOTS:
	/// Check for property changes.
	void updateProperties();

Q_SIGNALS:
	/// Properties have changed.
	void propertiesChanged();
	/// Propagates property update calls.
	void propagateUpdateProperties();
};

#endif
