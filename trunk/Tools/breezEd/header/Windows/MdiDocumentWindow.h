#ifndef MDIDOCUMENTWINDOW_H
#define MDIDOCUMENTWINDOW_H

#include <QtWidgets/QMdiSubWindow>

#include "Documents/AbstractDocument.h"

class MdiDocumentWindow : public QMdiSubWindow
{
	Q_OBJECT

private:
	DocumentReference<AbstractDocument> m_pDocument;

protected:
	/// Intercepts the close event.
	virtual void closeEvent(QCloseEvent *pEvent);

public:
	/// Constructor.
	MdiDocumentWindow(AbstractDocument *pDocument, QWidget *pParent = nullptr , Qt::WindowFlags flags = 0);
	/// Destructor.
	virtual ~MdiDocumentWindow();

	/// Gets the document.
	AbstractDocument* document() const { return m_pDocument; }

public Q_SLOTS:
	/// Updates the window title
	virtual void updateWindowTitle();
};

#endif
