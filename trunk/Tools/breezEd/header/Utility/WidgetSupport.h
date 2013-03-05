#ifndef WIDGETSUPPORT_H
#define WIDGETSUPPORT_H

#include "breezEd.h"
#include <QtWidgets/QAbstractButton>

/// Mirrors check box state in text form.
class TrueFalseBinder : public QObject
{
	Q_OBJECT

private:
	QAbstractButton *m_button;

private Q_SLOTS:
	void toggled()
	{
		m_button->setText( m_button->isChecked() ? tr("true") : tr("false") );
	}

public:
	TrueFalseBinder(QAbstractButton *checkBox)
		: QObject(checkBox),
		m_button(checkBox)
	{
		connect(m_button, &QAbstractButton::toggled, this, &TrueFalseBinder::toggled);
	}
};

#endif