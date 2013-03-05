#include "stdafx.h"
#include "Utility/Checked.h"

#include <QtCore/QCoreApplication>

#include "Utility/Strings.h"

// Converts exceptions to message box.
LEAN_NOLTINLINE void exceptionToMessageBox(const QString &givenTitle, const QString &givenText)
{
	QString defaultTitle, defaultText;

	if (givenTitle.isEmpty())
		defaultTitle = QCoreApplication::translate("Editor", "Unexpected error");
	if (givenText.isEmpty())
		defaultText = QCoreApplication::translate("Editor", "An unexpected error occurred.");
	
	const QString &title = (!givenTitle.isEmpty()) ? givenTitle : defaultTitle;
	const QString &text = (!givenText.isEmpty()) ? givenText : defaultText;

	try
	{
		throw;
	}
	catch (const std::exception &error)
	{
		QMessageBox msg;
		msg.setIcon(QMessageBox::Critical);
		msg.setWindowTitle(title);
		msg.setText(text);
		msg.setInformativeText( toQt(error.what()) );
		msg.exec();
	}
	catch (...)
	{
		QMessageBox::critical(nullptr, title, text);
	}
}
