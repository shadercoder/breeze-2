#ifndef SETCOMPONENTCOMMAND_H
#define SETCOMPONENTCOMMAND_H

#include <QtGui/QUndoCommand>

#include <beCore/beReflectedComponent.h>

#include <lean/smart/cloneable_obj.h>
#include <lean/containers/any.h>

/// Set component command class.
class SetComponentCommand : public QUndoCommand
{
private:
	beCore::ReflectedComponent *m_pReflectedComponent;
	uint4 m_componentIdx;

	lean::cloneable_obj<lean::any> m_previousComponent;
	lean::cloneable_obj<lean::any> m_component;

	bool m_bIgnoreOnce;

public:
	/// Constructor.
	SetComponentCommand(beCore::ReflectedComponent *pReflectedComponent, uint4 componentIdx,
		const lean::any &component, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~SetComponentCommand();

	/// Reverts the property.
	void undo();
	/// Changes the property.
    void redo();

	/// Parent component.
	beCore::ReflectedComponent* reflectedComponent() const { return m_pReflectedComponent; }
	/// Component.
	uint4 componentIdx() const { return m_componentIdx; }
};

#endif
