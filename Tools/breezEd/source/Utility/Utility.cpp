#include "stdafx.h"
#include "Utility/Checked.h"

#include <QtCore/QCoreApplication>

#include "Utility/Strings.h"

// Converts exceptions to message box.
LEAN_NOLTINLINE void exceptionToMessageBox(QString text, QString title)
{
	if (text.isEmpty())
		text = QCoreApplication::translate("Editor", "An unexpected error occurred.");
	if (title.isEmpty())
		title = QCoreApplication::translate("Editor", "Unexpected error");

	try
	{
		throw;
	}
	catch (const std::exception &error)
	{
		QMessageBox msg;
		msg.setIcon(QMessageBox::Critical);
		msg.setWindowTitle(title);
		msg.setText(title);
		msg.setInformativeText( toQt(error.what()) );
		msg.exec();
	}
	catch (...)
	{
		QMessageBox::critical( nullptr,
				text,
				title
			);
	}
}
