#include "stdafx.h"
#include "Docking/DockContainer.h"
#include "Docking/DockGroup.h"
#include "Docking/DockWidget.h"

#include <QtCore/QStringBuilder>

#include <lean/io/numeric.h>

#include "Utility/UI.h"

namespace
{

struct Dock
{
	QString name;
	DockWidget *widget;
	bool bActive;
	bool bMakeFront;

	Dock(QString name)
		: name(LEAN_MOVE(name)),
		widget(),
		bActive(),
		bMakeFront() { }
	Dock(DockWidget *widget)
		: name(widget->objectName()),
		widget(widget),
		bActive(false),
		bMakeFront() { }
};

typedef DockContainer::Split Split;

template <class T>
QSharedPointer<T> makeShared(T *p) { return QSharedPointer<T>(p); }

} // namespace

struct DockContainer::Split
{
	float percentage;
	Qt::Orientation orientation;
	bool bUndecorated;

	// Either
	typedef QLinkedList< QSharedPointer<Split> > nodes_t;
	nodes_t nodes;
	QSplitter *splitter;

	// Or
	typedef QLinkedList< QSharedPointer<Dock> > docks_t;
	docks_t docks;
	DockGroup *tabs;
	
	Split(Qt::Orientation orient, float perct)
		: percentage(perct),
		orientation(orient),
		bUndecorated(false),
		splitter(),
		tabs() { }
};

const QEvent::Type DockLayoutChangedEventType = static_cast<QEvent::Type>( QEvent::registerEventType() );

// Constructor.
DockContainer::DockContainer(QWidget *pParent)
	: QFrame(pParent),
	m_center( new DockWidget(this) ),
	m_split( new Split(Qt::Horizontal, 1.0f) ),
	m_rubberBand( new QRubberBand(QRubberBand::Rectangle) ),
	m_bTrackChildren( false ),
	m_dockLogicRevision( 0 ),
	m_dockUIRevision( 0 )
{
	this->setObjectName("DockContainer");
	this->setLayout( new QVBoxLayout() );
	this->layout()->setMargin(0);

	m_nestedSplitterName = this->objectName() + "_NestedSplitter";
	m_center->setObjectName(this->objectName() + "_CenterArea");
	
	// Logical split setup
	QSharedPointer<Split> centerDock( new Split(Qt::Horizontal, 1.0f) );
	centerDock->docks.push_back( makeShared( new Dock(m_center) ) );
	centerDock->bUndecorated = true;
	m_split->nodes.push_back(centerDock);

	updateWidgetTree();

	// Start tracking events
	m_bTrackChildren = true;
}

// Destructor.
DockContainer::~DockContainer()
{
	// Stop tracking events, let Qt clean up
	m_bTrackChildren = false;
}

// Sets the central widget.
void DockContainer::setCentralWidget(QWidget *widget)
{
	m_center->setWidget(widget);
}

namespace
{

QByteArray dockToString(const Dock &dock)
{
	return dock.name.toUtf8() + '\0'
		+ (dock.bActive ? "av" : "ah")
		+ (dock.bMakeFront || dock.widget && !dock.widget->isHidden() ? "fv" : "fh");
}

QByteArray splitToString(const Split &split)
{
	QByteArray result = "p" + QByteArray::number(split.percentage)
		+ ";o" + ((split.orientation == DockOrientation::Horizontal) ? "h" : "v")
		+ (split.bUndecorated ? "du" : "");

	for (Split::nodes_t::const_iterator it = split.nodes.begin(), itEnd = split.nodes.end(); it != itEnd; ++it)
		result += "(" + splitToString(**it) + ")";
	for (Split::docks_t::const_iterator it = split.docks.begin(), itEnd = split.docks.end(); it != itEnd; ++it)
		result += "[" + dockToString(**it) + "]";

	return result;
}

QSharedPointer<Dock> dockFromString(const char* &it, const char* itEnd)
{
	const char* itNameBegin = it;
	while (it != itEnd && *it != '\0')
		++it;
	const char* itNameEnd = it;
	QSharedPointer<Dock> dock( new Dock( QString::fromUtf8(itNameBegin, itNameEnd - itNameBegin) ) );

	while (it != itEnd && *it != ']')
	{
		if (*it == 'a' && ++it != itEnd)
			dock->bActive = (*it++ == 'v');
		else if (*it == 'f' && ++it != itEnd)
			dock->bMakeFront = (*it++ == 'v');
		else
			++it;
	}

	if (it != itEnd)
		++it;

	return dock;
}

QSharedPointer<Split> splitFromString(const char* &it, const char* itEnd)
{
	QSharedPointer<Split> split( new Split(Qt::Horizontal, 1.0f) );

	while (it != itEnd && *it != ')')
	{
		if (*it == 'p')
			it = lean::char_to_float(it + 1, itEnd, split->percentage);
		else if (*it == 'o' && ++it != itEnd)
			split->orientation = (*it++ == 'v') ? Qt::Vertical : Qt::Horizontal;
		else if (*it == 'd' && ++it != itEnd)
			split->bUndecorated = (*it++ == 'u');
		else if (*it == '(')
			split->nodes.push_back( splitFromString(++it, itEnd) );
		else if (*it == '[')
			split->docks.push_back( dockFromString(++it, itEnd) );
		else
			++it;
	}

	if (it != itEnd)
		++it;

	return split;
}

} // namespace

// Serializes the current state into a string.
QByteArray DockContainer::saveState() const
{
	return splitToString(*m_split);
}

namespace
{

Split* findLeaf(Split *root, QWidget *dock, Split::docks_t::iterator *pDockedIt = nullptr,
				Split **pSplit = nullptr, Split::nodes_t::iterator *pLeafIt = nullptr, bool bPrecise = false)
{
	if (pSplit)
		*pSplit = nullptr;

	for(Split::nodes_t::iterator it = root->nodes.begin(), itEnd = root->nodes.end(); it != itEnd; ++it)
		if (Split *leaf = findLeaf(it->data(), dock, pDockedIt, pSplit, pLeafIt, bPrecise))
		{
			if (pSplit && !*pSplit)
			{
				*pSplit = root;
				if (pLeafIt)
					*pLeafIt = it;
			}
			return leaf;
		}

	for(Split::docks_t::iterator it = root->docks.begin(), itEnd = root->docks.end(); it != itEnd; ++it)
		if ((bPrecise) ? dock == it->data()->widget : it->data()->widget && isChildOf(dock, it->data()->widget))
		{
			if (pDockedIt)
				*pDockedIt = it;
			return root;
		}

	return nullptr;
}

Split* findLeafByChild(Split *root, QWidget *widget)
{
	for(Split::nodes_t::iterator it = root->nodes.begin(), itEnd = root->nodes.end(); it != itEnd; ++it)
		if (Split *leaf = findLeafByChild(it->data(), widget))
			return leaf;

	if (root->tabs && isChildOf(widget, root->tabs))
		return root;
	else
		for(Split::docks_t::iterator it = root->docks.begin(), itEnd = root->docks.end(); it != itEnd; ++it)
			if (it->data()->widget && isChildOf(widget, it->data()->widget))
				return root;

	return nullptr;
}

Dock* findPlaceholder(Split *root, const QString &name, Split **pLeaf)
{
	for(Split::nodes_t::const_iterator it = root->nodes.begin(), itEnd = root->nodes.end(); it != itEnd; ++it)
		if (Dock *dock = findPlaceholder(it->data(), name, pLeaf))
			return dock;

	for(Split::docks_t::const_iterator it = root->docks.begin(), itEnd = root->docks.end(); it != itEnd; ++it)
	{
		Dock *dock = it->data();

		if (!dock->widget && dock->name == name)
		{
			if (pLeaf)
				*pLeaf = root;
			return dock;
		}
	}

	return nullptr;
}

QSharedPointer<Split> simplifyTree(const QSharedPointer<Split> &root, QWidget *newParent)
{
	bool bKeep = !root->docks.empty() || root->nodes.size() > 1;
	QWidget *newChildParent = (bKeep) ? root->splitter : newParent;

	for(Split::nodes_t::iterator it = root->nodes.end(); it-- != root->nodes.begin(); )
		if (QSharedPointer<Split> child = simplifyTree(*it, newChildParent))
			*it = child;
		else
			it = root->nodes.erase(it);

	if (!root->docks.empty() || root->nodes.size() > 1)
		return root;
	else if (!root->nodes.empty())
	{
		// NOTE: UI has to be up-to-date BEFORE simplification, i.e. simplification does not affect UI
		LEAN_ASSERT(root->splitter == nullptr);
		// Simply omit this node by returning only child
		return root->nodes.front();
	}
	else
	{
		// NOTE: UI has to be up-to-date BEFORE simplification, i.e. simplification does not affect UI
		LEAN_ASSERT(root->splitter == nullptr);
		LEAN_ASSERT(root->tabs == nullptr);
		// Nothing left, omit
		return QSharedPointer<Split>();
	}
}

void simplifyTree(Split &root, QWidget *parent)
{
	for(Split::nodes_t::iterator itBegin = root.nodes.begin(), it = root.nodes.end(); it-- != itBegin; )
		if (QSharedPointer<Split>  child = simplifyTree(*it, parent))
			*it = child;
		else
			it = root.nodes.erase(it);
}

QWidget* updateWidgetTree(Split &root, DockContainer *container, const QString &nestedSplitterName, bool bKeepSplitter = false)
{
	// TODO: Collect state

	QWidget *result = nullptr;

	// Nodes
	{
		QVector<QWidget*> activeChildren;

		for(Split::nodes_t::iterator it = root.nodes.begin(), itEnd = root.nodes.end(); it != itEnd; ++it)
			if (QWidget *child = updateWidgetTree(**it, container, nestedSplitterName))
				activeChildren.push_back(child);

		if (activeChildren.size() > 1 || bKeepSplitter)
		{
			if (!root.splitter)
			{
				root.splitter = new QSplitter(root.orientation, container);
				root.splitter->setObjectName(nestedSplitterName);
			}
			result = root.splitter;

			// TODO: Filter common prefix
			Q_FOREACH(QWidget *child, activeChildren)
				root.splitter->addWidget(child);
		}
		else
		{
			// Simply pass single childs through
			if (activeChildren.size() == 1)
			{
				result = activeChildren[0];

				if (isChildOf(result, root.splitter))
					result->setParent(container);
			}

			if (root.splitter)
			{
				delete root.splitter;
				root.splitter = nullptr;
			}
		}
	}

	// Docks / tabs
	{
		LEAN_ASSERT (root.docks.empty() || !result);

		QVector<QWidget*> activeChildren;
		QWidget *frontChild = nullptr;
		
		for(Split::docks_t::iterator it = root.docks.begin(), itEnd = root.docks.end(); it != itEnd; ++it)
		{
			Dock &dock = **it;

			if (QWidget *child = dock.widget)
			{
				dock.bActive |= !child->isHidden();

				if (dock.bActive)
				{
					activeChildren.push_back(child);
					if (dock.bMakeFront)
					{
						frontChild = child;
						// Reset
						dock.bMakeFront = false;
					}
				}
				else
					child->setParent(container);
			}
		}

		if (activeChildren.size() >= 1 && !root.bUndecorated)
		{
			if (!root.tabs)
			{
				root.tabs = new DockGroup(container);
				QObject::connect(root.tabs, &DockGroup::dockDragged, container, &DockContainer::dockDragged);
				QObject::connect(root.tabs, &DockGroup::dockDropped, container, &DockContainer::dockDropped, Qt::QueuedConnection);
			}
			result = root.tabs;

			int tabPos = 0;

			// TODO: Filter common prefix
			Q_FOREACH(QWidget *child, activeChildren)
				root.tabs->addWidget(child, tabPos++);

			if (frontChild)
				root.tabs->setCurrentTab(frontChild);
		}
		else
		{
			// Simply pass single undecorated childs through
			if (activeChildren.size() == 1)
			{
				result = activeChildren[0];

				if (isChildOf(result, root.tabs))
					result->setParent(container);
			}

			if (root.tabs)
			{
				delete root.tabs;
				root.tabs = nullptr;
			}
		}
	}

	return result;
}

/// Tries to emplace the given dock.
bool tryEmplace(Split &root, DockWidget *dock, bool bActive)
{
	if (Dock *place = findPlaceholder(&root, dock->objectName(), nullptr))
	{
		place->widget = dock;
		place->bActive = bActive;
		return true;
	}
	else
		return false;
}

/// Transfers all matching docks from the given source to the given destination tree.
void transfer(Split &source, Split &dest, QWidget *tmpParent)
{
	for(Split::nodes_t::const_iterator it = source.nodes.begin(), itEnd = source.nodes.end(); it != itEnd; ++it)
		transfer(**it, dest, tmpParent);

	for(Split::docks_t::const_iterator it = source.docks.begin(), itEnd = source.docks.end(); it != itEnd; ++it)
	{
		Dock &dock = **it;

		if (dock.widget)
		{
			bool bKeep = false;

			if (Dock *place = findPlaceholder(&dest, dock.name, nullptr))
			{
				place->widget = dock.widget;
				place->widget->setParent(tmpParent);
				bKeep = place->bActive;
			}

			if (!bKeep)
				dock.widget->close();

			dock.widget = nullptr;
		}
	}
}

// Removes the given dock.
void removeDock(Split &leaf, Split::docks_t::iterator itDocked, QWidget *newParent, DockContainer *pDisconnect, bool bPermanent)
{
	Dock &dock = **itDocked;

	if (dock.widget)
	{
		QObject::disconnect(dock.widget, nullptr, pDisconnect, nullptr);
		dock.widget->setParent(newParent);
	}
		
	if (bPermanent)
		leaf.docks.erase(itDocked);
	else
	{
		dock.widget = nullptr;
		dock.bActive = false;
	}
}

} // namespace

// Deserializes state from the given string.
void DockContainer::restoreState(const QByteArray &state)
{
	if (state.isEmpty())
		return;

	const char *it = state.data();
	QSharedPointer<Split> newState = splitFromString(it, it + state.size());
	
	// Transfer & get rid of old widgets
	updateWidgetTree();
	transfer(*m_split, *newState, this);
	updateWidgetTree();
	
	// Replace & construct new widgets
	delete m_split->splitter;
	m_split = newState;
	updateWidgetTree();
}

// Places the given dock widget next to the given dock widget.
void DockContainer::addDock(DockWidget *dock, DockPlacement::T placement, DockOrientation::T orientation, DockSide::T side, QWidget *neighbor, DockCompany::T company)
{
	bool bEmplaced = false;

	if (placement == DockPlacement::Emplace)
	{
		// Keep it where it is
		if (findLeaf(m_split.data(), dock))
			return;
		// Emplace in existing placeholder
		else 
			bEmplaced = ::tryEmplace(*m_split, dock, !dock->isHidden());
	}

	if (!bEmplaced)
	{
		Split *leaf = nullptr;
		Split::docks_t::iterator itDocked;
		Split *split;
		Split::nodes_t::iterator itLeaf;

		// Find entries for the given neighbor or the center widget, if neighbor not given/found
		if (neighbor)
			leaf = findLeaf(m_split.data(), neighbor, &itDocked, &split, &itLeaf);
		if (!leaf)
		{
			neighbor = m_center;
			leaf = findLeaf(m_split.data(), neighbor, &itDocked, &split, &itLeaf);
		}
		LEAN_ASSERT(leaf && split);

		// Keep center widget tab-free
		if (leaf->bUndecorated && company == DockCompany::Join)
			company = DockCompany::JoinNeighbor;

		// Move docked iterator to insertion position
		if (side == DockSide::After)
			++itDocked;

		// Try to find open neighbors
		if (company == DockCompany::JoinNeighbor)
		{
			// Insert at the end of the left neighbor, if existent & open
			if (side == DockSide::Before && itLeaf != split->nodes.begin() && !(itLeaf - 1)->data()->bUndecorated)
			{
				--itLeaf;
				leaf = itLeaf->data();
				itDocked = leaf->docks.end();
				company = DockCompany::Join;
			}
			// Insert at the front of the right neighbor, if existent & open
			else if (side == DockSide::After && itLeaf + 1 != split->nodes.end() && !(itLeaf + 1)->data()->bUndecorated)
			{
				++itLeaf;
				leaf = itLeaf->data();
				itDocked = leaf->docks.begin();
				company = DockCompany::Join;
			}
		}

		if (company != DockCompany::Join)
		{
			if (split->orientation != toQt(orientation))
			{
				QSharedPointer<Split> newSplit( new Split(toQt(orientation), leaf->percentage) );
				newSplit->nodes.push_back(*itLeaf);
				*itLeaf = newSplit;
				leaf->percentage = 1.0f;

				split = newSplit.data();
				itLeaf = split->nodes.begin();
			}

			// Move leaf iterator to insertion position
			if (side == DockSide::After)
				++itLeaf;

			// Insert new leaf
			itLeaf = split->nodes.insert(itLeaf, makeShared( new Split(toQt(orientation), 0.0f) ) );
			leaf = itLeaf->data();
			itDocked = leaf->docks.begin();
		}

		// ORDER: Create dock item before widget management screws internal state
		QSharedPointer<Dock> dockItem( new Dock(dock) );

		// NOTE: Handle re-insertions here to keep reference widget & iterators valid
		{
			Split::docks_t::iterator itDockedOld;
			Split *oldLeaf = findLeaf(m_split.data(), dock, &itDockedOld);

			if (oldLeaf)
			{
				// NOTE: Visibility no longer meaningful
				dockItem->bActive |= itDockedOld->data()->bActive;

				// Ignore re-insertion at the same position
				// NOTE: Always handle, iterator would become invalid on removal!
				if (itDockedOld == itDocked || itDockedOld + 1 == itDocked)
					return;
				// Otherwise, remove previous occurance
				else
					::removeDock(*oldLeaf, itDockedOld, this, this, true);
			}
		}

		// Insert new dock
		itDocked = leaf->docks.insert(itDocked, dockItem);
	}

	dock->setParent(this);
	connect(dock, &DockWidget::parentChanged, this, &DockContainer::dockReparented);
	connect(dock, &DockWidget::destroyed, this, &DockContainer::dockDestroyed);
	connect(dock, &DockWidget::shown, this, &DockContainer::dockShown, Qt::QueuedConnection);
	connect(dock, &DockWidget::closed, this, &DockContainer::dockHidden, Qt::QueuedConnection);

	lazyUpdateWidgetTree();
}

// Removes the given dock.
void DockContainer::removeDock(DockWidget *dock, bool bPermanent)
{
	Split::docks_t::iterator itDocked;
	if (Split *leaf = findLeaf(m_split.data(), dock, &itDocked))
	{
		::removeDock(*leaf, itDocked, nullptr, this, bPermanent);
		// TODO: Simplify tree
		lazyUpdateWidgetTree();
	}
}

// Checks if the given two widgets share the same group.
bool DockContainer::sameGroup(QWidget *a, QWidget *b) const
{
	Split *leafA = findLeaf(m_split.data(), a);
	Split *leafB = findLeaf(m_split.data(), b);

	return leafA && leafB && leafA == leafB;
}

// Raises the given dock widget when the UI has been updated.
void DockContainer::raiseDock(DockWidget *dock)
{
	Split::docks_t::iterator itDocked;
	if (Split *leaf = findLeaf(m_split.data(), dock, &itDocked))
	{
		itDocked->data()->bMakeFront = true;
		dock->show();
		lazyUpdateWidgetTree();
	}
}

// Lazily updates widgets in the tree structure.
void DockContainer::lazyUpdateWidgetTree()
{
	++m_dockLogicRevision;
	QApplication::postEvent(this, new QEvent(DockLayoutChangedEventType));
}

// Creates/updates widgets in the tree structure.
void DockContainer::updateWidgetTree()
{
	int logicRevision = m_dockLogicRevision;

	this->setUpdatesEnabled(false);
	this->layout()->addWidget( ::updateWidgetTree(*m_split, this, m_nestedSplitterName, true) );
	this->setUpdatesEnabled(true);

	m_dockUIRevision = logicRevision;

	simplifyTree(*m_split, this);
}

namespace
{

QRect mapToGlobal(QWidget *widget, QRect rect)
{
	rect.moveTo( widget->mapToGlobal( rect.topLeft() ) );
	return rect;
}

DockCompany::T dockTargetFlags(QRect rect, QPoint screenPos, bool bMustSplit, DockOrientation::T &orientation, DockSide::T &side, QRect *pTargetRect = nullptr)
{
	DockCompany::T company = DockCompany::Join;

	if (pTargetRect)
		*pTargetRect = rect;
	
	QPoint center = rect.center();
	QPointF eccentricity = (screenPos - center);
	eccentricity.setX(2.0f * eccentricity.x() / rect.width());
	eccentricity.setY(2.0f * eccentricity.y() / rect.height());

	float eccentricityLimit = (bMustSplit) ? 0.0f : 0.5f;

	if (abs(eccentricity.x()) > abs(eccentricity.y()))
	{
		orientation = DockOrientation::Horizontal;

		if (eccentricity.x() < -eccentricityLimit)
		{
			company = DockCompany::Single;
			side = DockSide::Before;
			if (pTargetRect)
				pTargetRect->setRight(center.x());
		}
		else if (eccentricity.x() >= eccentricityLimit)
		{
			company = DockCompany::Single;
			side = DockSide::After;
			if (pTargetRect)
				pTargetRect->setLeft(center.x());
		}
	}
	else
	{
		orientation = DockOrientation::Vertical;

		if (eccentricity.y() < -eccentricityLimit)
		{
			company = DockCompany::Single;
			side = DockSide::Before;
			if (pTargetRect)
				pTargetRect->setBottom(center.y());
		}
		else if (eccentricity.y() >= eccentricityLimit)
		{
			company = DockCompany::Single;
			side = DockSide::After;
			if (pTargetRect)
				pTargetRect->setTop(center.y());
		}
	}

	return company;
}

} // namespace

// Updates drag feedback.
void DockContainer::dockDragDrop(QWidget *dockWdgt, QPoint screenPos, bool bDropped)
{
	QWidget *targetWidget = nullptr;
	QWidget *targetTab = nullptr;
	bool bAppendOrSplit = true;
	bool bMustSplit = false;

	DockWidget *dock = LEAN_ASSERT_NOT_NULL( qobject_cast<DockWidget*>(dockWdgt) );

	{
		QWidget *cursorWidget = QApplication::widgetAt(screenPos);

		if (Split *leaf = findLeafByChild(m_split.data(), cursorWidget))
		{
			if (leaf->bUndecorated)
			{
				targetWidget = targetTab = leaf->docks.front()->widget;
				bMustSplit = true;
			}
			else
			{
				DockGroup *targetDock = leaf->tabs;

				targetWidget = targetDock;
				if (targetTab = targetDock->tabFromWidget(cursorWidget))
				{
					targetWidget = cursorWidget;
					bAppendOrSplit = false;
				}
				else
					targetTab = targetDock->currentTab();
			}
		}
	}

	bool bHighlight = !bDropped;
	bool bAccepted = false;

	if (targetTab)
	{
		DockCompany::T splitCompany = DockCompany::Join;
		DockOrientation::T splitOrient = DockOrientation::Horizontal;
		DockSide::T splitSide = DockSide::After;

		QRect targetRect = ::mapToGlobal(targetWidget->parentWidget(), targetWidget->geometry());

		if (bAppendOrSplit)
			splitCompany = dockTargetFlags(targetRect, screenPos, bMustSplit, splitOrient, splitSide, &targetRect);
		else
		{
			if (screenPos.x() < targetRect.center().x())
			{
				splitSide = DockSide::Before;
				targetRect.translate(-targetRect.width() / 2, 0);
			}
			else
			{
				splitSide = DockSide::After;
				targetRect.translate(targetRect.width() / 2, 0);
			}
		}

		m_rubberBand->setGeometry(targetRect);

		if (bDropped)
		{
			addDock(dock, DockPlacement::Independent, splitOrient, splitSide, targetTab, splitCompany);
			bAccepted = true;
		}
	}
	else
		m_rubberBand->setGeometry( dock->geometry().translated(screenPos - dock->pos()) );

	m_rubberBand->setVisible(bHighlight);

	if (bDropped && !bAccepted)
			QMessageBox::information(nullptr, tr("Unsupported"), tr("Free dock widgets currently unsupported"));
}

// The given dock was destroyed.
void DockContainer::dockDestroyed(DockWidget *dock)
{
	if (!m_bTrackChildren)
		return;

	Split::docks_t::iterator itDocked;
	if (Split *leaf = findLeaf(m_split.data(), dock, &itDocked, nullptr, nullptr, true))
	{
		itDocked->data()->widget = nullptr;

		lazyUpdateWidgetTree();
	}
}

// The dock parent has changed.
void DockContainer::dockReparented(DockWidget *dock)
{
	if (!m_bTrackChildren)
		return;

	Split::docks_t::iterator itDocked;
	if (Split *leaf = findLeaf(m_split.data(), dock, &itDocked, nullptr, nullptr, true))
	{
		DockContainer *container = findParent<DockContainer*>(dock);

		// Detect re-parenting outside this container
		if (container != this)
		{
			QObject::disconnect(dock, nullptr, this, nullptr);
			itDocked->data()->widget = nullptr;

			lazyUpdateWidgetTree();
		}
	}
}

// The given dock was shown.
void DockContainer::dockShown(DockWidget *dock)
{
	if (!m_bTrackChildren)
		return;

	Split::docks_t::iterator itDocked;
	if (Split *leaf = findLeaf(m_split.data(), dock, &itDocked))
		// Detect changes!
		if (!itDocked->data()->bActive)
			lazyUpdateWidgetTree();
}

// The given dock was hidden.
void DockContainer::dockHidden(DockWidget *dock)
{
	if (!m_bTrackChildren)
		return;

	Split::docks_t::iterator itDocked;
	if (Split *leaf = findLeaf(m_split.data(), dock, &itDocked, nullptr, nullptr, true))
	{
		itDocked->data()->bActive = false;
		lazyUpdateWidgetTree();
	}
}

// Updates drag feedback.
void DockContainer::dockDragged(QWidget *dock, QPoint screenPos)
{
	if (m_bTrackChildren)
		dockDragDrop(dock, screenPos, false);
}

// Redocks dragged dock widgets.
void DockContainer::dockDropped(QWidget *dock, QPoint screenPos)
{
	if (m_bTrackChildren)
		dockDragDrop(dock, screenPos, true);
}

bool DockContainer::event(QEvent *event)
{
	if (event->type() == DockLayoutChangedEventType)
	{
		if (m_dockUIRevision != m_dockLogicRevision)
			updateWidgetTree();
		return true;
	}
	else
		return QFrame::event(event);
}
