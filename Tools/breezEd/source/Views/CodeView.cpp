#include "stdafx.h"
#include "Views/CodeView.h"

#include "Documents/CodeDocument.h"
#include "Modes/Mode.h"
#include "Modes/CodeViewModeState.h"

#include "Utility/CodeAutoFormatter.h"

#include "Utility/Checked.h"

// Constructor.
CodeView::CodeView(CodeDocument *pDocument, Mode *pDocumentMode, Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
	: AbstractView(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) ),
	m_pDocument( LEAN_ASSERT_NOT_NULL(pDocument) ),
	m_pViewMode( new Mode(LEAN_ASSERT_NOT_NULL(pDocumentMode)) )
{
	ui.setupUi(this);

	ui.codeEditor->setDocument(m_pDocument->textDocument());
	ui.codeEditor->setTabStopWidth(m_pDocument->textDocument()->indentWidth());

	// Set up view mode
	m_pViewMode->addState( new CodeViewModeState(this, m_pEditor, m_pViewMode) );

	new CodeAutoFormatter(ui.codeEditor);
}

// Destructor.
CodeView::~CodeView()
{
	m_pViewMode->exit();
}

// Activates this view.
void CodeView::activate()
{
	m_pViewMode->enter();
}
