#ifndef FILECREATIONPARAMETERWIDGET_H
#define FILECREATIONPARAMETERWIDGET_H

#include "CreationParameterWidget.h"
#include "BrowseWidget.h"

/// File creation parameter widget.
class FileCreationParameterWidget : public CreationParameterWidget
{
	Q_OBJECT

private:
	BrowseWidget *m_pWidget;

	QString m_name;

private Q_SLOTS:
	/// Browses for a file.
	void browse();

protected:
	/// Browses for a file.
	virtual QString onBrowse(const QString &path);

public:
	/// Constructor.
	FileCreationParameterWidget(const QString &name, Editor *pEditor, QWidget *pParent = nullptr);
	/// Destructor.
	~FileCreationParameterWidget();

	/// Sets the creation parameter in the given set.
	void setParameters(beCore::Parameters &parameters, SceneDocument &document) const;

	/// Gets the parameter name.
	QString name() const { return m_name; }

	/// Gets the widget.
	BrowseWidget* widget();
	/// Gets the widget.
	const BrowseWidget* widget() const;
};

#endif
