#include "stdafx.h"
#include "Widgets/BrowseWidget.h"

#include "Utility/Checked.h"

// Constructor.
BrowseWidget::BrowseWidget( QWidget *pParent, Qt::WindowFlags flags)
	: QWidget(pParent, flags)
{
	ui.setupUi(this);

	ui.browseButton->setFocus();
	setFocusProxy(ui.browseButton);

	checkedConnect(ui.browseButton, SIGNAL(clicked()), this, SIGNAL(browse()));
	checkedConnect(ui.pathEdit, SIGNAL(cursorPositionChanged(int, int)), this, SIGNAL(editingStarted()));
	checkedConnect(ui.pathEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
	checkedConnect(ui.pathEdit, SIGNAL(editingFinished()), this, SIGNAL(pathSelected()));
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

	Q_EMIT pathSelected();
}

// Gets the path.
QString BrowseWidget::path() const
{
	return ui.pathEdit->text();
}
