/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beCoreInternal/stdafx.h"
#include "beCore/beTextSerialization.h"
#include <algorithm>

namespace beCore
{

// Constructor.
TextSerialization::TextSerialization(TypeIndex *pTypeIndex)
	: m_pTypeIndex( LEAN_ASSERT_NOT_NULL(pTypeIndex) )
{
}

// Constructor.
TextSerialization::~TextSerialization()
{
}

// Adds the given serializer to this serialization manager.
void TextSerialization::AddSerializer(const TextSerializer *pSerializer)
{
	LEAN_ASSERT(pSerializer);

	uint4 typeID = m_pTypeIndex->GetID(pSerializer->GetType());

	if (typeID >= m_serializers.size())
		m_serializers.resize(typeID + 1);

	m_serializers[typeID] = pSerializer;
}

// Removes the given serializer from this serialization manager.
bool TextSerialization::RemoveSerializer(const TextSerializer *pSerializer)
{
	return RemoveSerializer(pSerializer->GetType());
}

// Removes the given serializer from this serialization manager.
bool TextSerialization::RemoveSerializer(const TypeIndex &typeIndex, uint4 typeID)
{
	if (&typeIndex == m_pTypeIndex)
	{
		if (typeID < m_serializers.size())
		{
			m_serializers[typeID] = nullptr;
			return true;
		}
		else
			return false;
	}
	else
		return RemoveSerializer(typeIndex.GetName(typeID));
}

// Removes the given serializer from this serialization manager.
bool TextSerialization::RemoveSerializer(const utf8_ntri &type)
{
	return RemoveSerializer(*m_pTypeIndex, m_pTypeIndex->GetID(type));
}

// Gets the global text serialization.
TextSerialization& GetTextSerialization()
{
	static TextSerialization serialization(&GetTypeIndex());
	return serialization;
}

} // namespace
