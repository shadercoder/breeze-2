#ifndef CODEAUTOFORMATTER_H
#define CODEAUTOFORMATTER_H

#include "breezEd.h"

#include <QtGui/QPlainTextEdit>
#include "Utility/Checked.h"

LEAN_INLINE bool isBreak(QChar ch)
{
	QChar::Category cat = ch.category();
	return (cat == QChar::Separator_Line || cat == QChar::Separator_Paragraph);
}

LEAN_INLINE bool isSpaceOrControl(QChar ch)
{
	QChar::Category cat = ch.category();
	return (cat == QChar::Separator_Space || cat == QChar::Other_Control);
}

/// Code auto formatter.
class CodeAutoFormatter : public QObject
{
	Q_OBJECT

private:
	QPlainTextEdit *m_pTextEdit;

public:
	/// Constructor.
	CodeAutoFormatter(QPlainTextEdit *pTextEdit)
		: QObject( pTextEdit ),
		m_pTextEdit( LEAN_ASSERT_NOT_NULL(pTextEdit) )
	{
		checkedConnect(m_pTextEdit, SIGNAL(blockCountChanged(int)), this, SLOT(maybeIndentNewLine()));
	}

public Q_SLOTS:
	/// Indents new lines in the same way as previous lines.
	void maybeIndentNewLine()
	{
		QTextCursor cursor = m_pTextEdit->textCursor();

		// Indention requires vaild document
		// Never indent during undo / redo
		// Never replace selections by indention
		if (m_pTextEdit->document() && !m_pTextEdit->document()->isRedoAvailable() && !cursor.hasSelection())
		{
			QChar character = m_pTextEdit->document()->characterAt(cursor.position() - 1);
			QChar nextCharacter = m_pTextEdit->document()->characterAt(cursor.position());
			
			// Check if beginning of line & no indention
			if (isBreak(character) && nextCharacter.category() != QChar::Separator_Space)
			{
				// Extract previous line (empty if first)
				QString prevLine = cursor.block().previous().text();
				
				int prevLineIndentEnd = 0;

				// Extract previous indention
				// IMPORTANT: constData() guaranteed to be null-terminated
				while (isSpaceOrControl(prevLine.constData()[prevLineIndentEnd]))
					++prevLineIndentEnd;

				// Repeat indention
				prevLine.truncate(prevLineIndentEnd);
				cursor.insertText(prevLine);
			}
		}
	}
};

#endif