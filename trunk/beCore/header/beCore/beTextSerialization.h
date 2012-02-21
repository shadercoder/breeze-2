/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_TEXT_SERIALIZATION
#define BE_CORE_TEXT_SERIALIZATION

#include "beCore.h"
#include <lean/tags/noncopyable.h>
#include "beTypeIndex.h"
#include "beTextSerializer.h"
#include <vector>

namespace beCore
{

/// Text serialization manager.
class TextSerialization : public lean::noncopyable
{
private:
	TypeIndex *m_pTypeIndex;

	typedef std::vector<const TextSerializer*> serializer_vector;
	serializer_vector m_serializers;

public:
	/// Constructor.
	BE_CORE_API TextSerialization(TypeIndex *pTypeIndex);
	/// Constructor.
	BE_CORE_API ~TextSerialization();

	/// Adds the given serializer to this serialization manager.
	BE_CORE_API void AddSerializer(const TextSerializer *pSerializer);
	/// Removes the given serializer from this serialization manager.
	BE_CORE_API bool RemoveSerializer(const TextSerializer *pSerializer);
	/// Removes the given serializer from this serialization manager.
	BE_CORE_API bool RemoveSerializer(const TypeIndex &typeIndex, uint4 typeID);
	/// Removes the given serializer from this serialization manager.
	BE_CORE_API bool RemoveSerializer(const utf8_ntri &type);

	/// Gets a serializer for the given serializable type, if available, returns nullptr otherwise.
	LEAN_INLINE const TextSerializer* GetSerializer(uint4 typeID) const
	{
		return (typeID < m_serializers.size())
			? m_serializers[typeID]
			: nullptr;
	}
	/// Gets a serializer for the given serializable type, if available, returns nullptr otherwise.
	const TextSerializer* GetSerializer(const TypeIndex &typeIndex, uint4 typeID) const
	{
		return (&typeIndex == m_pTypeIndex)
			? GetSerializer( typeID )
			: GetSerializer( typeIndex.GetName(typeID) );
	}
	/// Gets a serializer for the given serializable type, if available, returns nullptr otherwise.
	LEAN_INLINE const TextSerializer* GetSerializer(const utf8_ntri &type) const
	{
		return GetSerializer( m_pTypeIndex->GetID(type) );
	}

	/// Gets the type index.
	LEAN_INLINE TypeIndex* GetTypeIndex() const { return m_pTypeIndex; };
};

/// Gets the global text serialization.
BE_CORE_API TextSerialization& GetTextSerialization();

} // namespace

#endif