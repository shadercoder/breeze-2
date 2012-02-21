#include "stdafx.h"
#include "Widgets/BrowseWidget.h"

#include "Utility/Checked.h"

// Constructor.
BrowseWidget::BrowseWidget( QWidget *pParent, Qt::WFlags flags)
	: QWidget(pParent, flags)
{
	ui.setupUi(this);

	checkedConnect(ui.browseButton, SIGNAL(clicked()), this, SIGNAL(browse()));
}

// Destructor.
BrowseWidget::~BrowseWidget()
{
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
