#ifndef DOCKCONTAINER_H
#define DOCKCONTAINER_H

#include <QtWidgets/QWidget>
#include <QtCore/QScopedPointer>

#include "DockWidget.h"

struct DockSide
{
	enum T
	{
		Before,
		After
	};
};
struct DockOrientation
{
	enum T
	{
		Horizontal = Qt::Horizontal,
		Vertical = Qt::Vertical
	};
};
inline Qt::Orientation toQt(DockOrientation::T orient) { return static_cast<Qt::Orientation>(orient); }
struct DockCompany
{
	enum T
	{
		Single,
		JoinNeighbor,
		Join
	};
};
struct DockPlacement
{
	enum T
	{
		Independent,
		Emplace
	};
};

/// Dock container.
class DockContainer : public QFrame
{
	Q_OBJECT

public:
	struct Split;

private:
	DockWidget *m_center;
	QSharedPointer<Split> m_split;
	QString m_nestedSplitterName;
	QScopedPointer<QRubberBand> m_rubberBand;

	int m_dockLogicRevision;
	int m_dockUIRevision;
	bool m_bTrackChildren;

	/// Tries to emplace the given dock.
	bool tryEmplace(QWidget *dock);

	/// The given dock was destroyed.
	void dockDestroyed(DockWidget *dock);
	/// The dock parent has changed.
	void dockReparented(DockWidget *dock);
	/// The given dock was shown.
	void dockShown(DockWidget *dock);
	/// The given dock was hidden.
	void dockHidden(DockWidget *dock);

	/// Creates/updates widgets in the tree structure.
	void updateWidgetTree();
	/// Lazily updates widgets in the tree structure.
	void lazyUpdateWidgetTree();
	/// Updates drag feedback.
	void dockDragDrop(QWidget *dock, QPoint screenPos, bool bDropped);

protected:
	bool event(QEvent *event);

public:
	/// Constructor.
	DockContainer(QWidget *pParent = nullptr);
	/// Destructor.
	~DockContainer();

	/// Sets the central widget.
	void setCentralWidget(QWidget *widget);
	/// Places the given dock widget next to the given dock widget.
	void addDock(DockWidget *dock, DockPlacement::T placement,
		DockOrientation::T orientation = DockOrientation::Horizontal,
		DockSide::T side = DockSide::After,
		QWidget *neighbor = nullptr,
		DockCompany::T company = DockCompany::Join);
	/// Removes the given dock.
	void removeDock(DockWidget *dock, bool bPermanent = false); 
	/// Checks if the given two widgets share the same group.
	bool sameGroup(QWidget *a, QWidget *b) const;

	/// Raises the given dock widget when the UI has been updated.
	void raiseDock(DockWidget *dock);

	/// Gets the name of nested splitters.
	QString nestedSplitterName() const { return m_nestedSplitterName; }
	/// Gets the name of the center widget.
	QString centerName() const { return m_center->objectName(); }

	/// Updates drag feedback.
	void dockDragged(QWidget *dock, QPoint screenPos);
	/// Redocks dragged dock widgets.
	void dockDropped(QWidget *dock, QPoint screenPos);

	/// Serializes the current state into a string.
	QByteArray saveState() const;
	/// Deserializes state from the given string.
	void restoreState(const QByteArray &state);
};

#endif
