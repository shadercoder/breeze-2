/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_PROPERTY_VISITOR
#define BE_CORE_PROPERTY_VISITOR

#include "beCore.h"
#include "bePropertyProvider.h"
#include "beDataVisitor.h"

namespace beCore
{

/// Property visitor.
class LEAN_INTERFACE PropertyVisitor
{
protected:
	~PropertyVisitor() { }

public:
	/// Visits the given values.
	virtual bool Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, void *values)
	{
		Visit(provider, propertyID, desc, const_cast<const void*>(values));
		return false;
	}
	/// Visits the given values.
	virtual void Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, const void *values) { }
};

/// Property data visitor adapter.
class PropertyDataVisitor : public PropertyVisitor
{
	DataVisitor *m_pVisitor;

public:
	/// Constructor.
	PropertyDataVisitor(DataVisitor &visitor)
		: m_pVisitor(&visitor) { }

	/// Visits the given values.
	bool Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, void *values)
	{
		return m_pVisitor->Visit(desc.TypeID, *desc.TypeInfo, values, desc.Count);
	}
	/// Visits the given values.
	void Visit(const PropertyProvider &provider, uint4 propertyID, const PropertyDesc &desc, const void *values)
	{
		m_pVisitor->Visit(desc.TypeID, *desc.TypeInfo, values, desc.Count);
	}
};

} // namespace

#endif