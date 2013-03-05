#include "stdafx.h"
#include "Windows/WindowsDialog.h"

#include <QtWidgets/QMdiSubWindow>
#include "Windows/MdiDocumentWindow.h"

#include "Utility/Checked.h"

// Constructor.
WindowsDialog::WindowsDialog(const QList<QWidget*> &windows, QWidget *pParent, Qt::WindowFlags flags)
	: QDialog(pParent, flags)
{
	ui.setupUi(this);

	assignWindows(windows);
}

// Destructor.
WindowsDialog::~WindowsDialog()
{
}

// Assigns new windows to this dialog.
void WindowsDialog::assignWindows(const QList<QWidget*> &windows)
{
	ui.windowList->clear();
	m_windows.clear();

	// Loop over windows
	Q_FOREACH (QWidget *pWindow, windows)
		// Add window to list
		ui.windowList->addItem(pWindow->windowTitle());

	m_windows = windows;

	selectionChanged();
}

// Reacts to selection changes.
void WindowsDialog::selectionChanged()
{
	int selectedItemCount = ui.windowList->selectedItems().count();

	ui.activateButton->setEnabled(selectedItemCount == 1);

	ui.saveButton->setEnabled(selectedItemCount >= 1);
	ui.closeButton->setEnabled(selectedItemCount >= 1);
}

// Activates the selected window.
void WindowsDialog::activateSelected()
{
	QList<QListWidgetItem*> selectedItems = ui.windowList->selectedItems();

	// Only one window at a time
	if (selectedItems.count() == 1)
	{
		int index = ui.windowList->row(selectedItems.front());

		if (0 <= index && index < m_windows.count())
		{
			QWidget *pWindow = m_windows.at(index);

			// NOTE: Might have been closed
			if (pWindow)
			{
				QMdiSubWindow *pMdiWindow = qobject_cast<QMdiSubWindow*>(pWindow);

				// Activate sub-window
				if (pMdiWindow && pMdiWindow->mdiArea())
					pMdiWindow->mdiArea()->setActiveSubWindow(pMdiWindow);
				else
					pWindow->setFocus();
			}
		}
	}
}

// Saves the selected window.
void WindowsDialog::saveSelected()
{
	QList<QListWidgetItem*> selectedItems = ui.windowList->selectedItems();

	Q_FOREACH (QListWidgetItem *pSelectedItem, selectedItems)
	{
		int index = ui.windowList->row(pSelectedItem);

		if (0 <= index && index < m_windows.count())
		{
			MdiDocumentWindow *pDocumentWindow = qobject_cast<MdiDocumentWindow*>(m_windows.at(index));

			// Ignore non-document windows (and closed ones!)
			if (pDocumentWindow)
			{
				// Save document
				pDocumentWindow->document()->save();

				// Update title
				pSelectedItem->setText(pDocumentWindow->windowTitle());
			}
		}
	}
}

// Closes the selected window.
void WindowsDialog::closeSelected()
{
	QList<QListWidgetItem*> selectedItems = ui.windowList->selectedItems();

	Q_FOREACH (QListWidgetItem *pSelectedItem, selectedItems)
	{
		int index = ui.windowList->row(pSelectedItem);

		if (0 <= index && index < m_windows.count())
		{
			QWidget *pWindow = m_windows.at(index);

			// Close
			// NOTE: Might have been closed
			if (pWindow && pWindow->close())
			{
				m_windows[index] = nullptr;
				pSelectedItem->setHidden(true);
				pSelectedItem->setSelected(false);
				pSelectedItem->setFlags(Qt::NoItemFlags);
			}
		}
	}
}
