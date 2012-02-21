/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_DATA_VISITOR
#define BE_CORE_DATA_VISITOR

#include "beCore.h"
#include <lean/type_info.h>

namespace beCore
{

/// Data visitor.
class DataVisitor
{
protected:
	~DataVisitor() { }

public:
	/// Visits the given values.
	virtual bool Visit(uint4 typeID, const lean::type_info &typeInfo, void *values, size_t count)
	{
		Visit(typeID, typeInfo, const_cast<const void*>(values), count);
		return false;
	}
	/// Visits the given values.
	virtual void Visit(uint4 typeID, const lean::type_info &typeInfo, const void *values, size_t count) { }
};

} // namespace

#endif