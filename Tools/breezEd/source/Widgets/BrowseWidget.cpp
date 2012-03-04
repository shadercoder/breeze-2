#include "stdafx.h"
#include "Widgets/BrowseWidget.h"

#include "Utility/Checked.h"

// Constructor.
BrowseWidget::BrowseWidget( QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags)
{
	ui.setupUi(this);

	ui.browseButton->setFocus();
	setFocusProxy(ui.browseButton);

	checkedConnect(ui.browseButton, SIGNAL(clicked()), this, SIGNAL(browse()));
	checkedConnect(ui.pathEdit, SIGNAL(cursorPositionChanged(int, int)), this, SIGNAL(editingStarted()));
	checkedConnect(ui.pathEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}

// Destructor.
BrowseWidget::~BrowseWidget()
{
}

// Installs the given event handler on all relevant child widgets.
void BrowseWidget::installFocusHandler(QObject *handler)
{
	ui.browseButton->installEventFilter(handler);
	ui.pathEdit->installEventFilter(handler);
}

// Sets the path.
void BrowseWidget::setPath(const QString &path)
{
	ui.pathEdit->setText(path);
}

// Gets the path.
QString BrowseWidget::path() const
{
	return ui.pathEdit->text();
}
