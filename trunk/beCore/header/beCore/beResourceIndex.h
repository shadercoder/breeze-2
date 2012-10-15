/*****************************************************/
/* breeze Engine Core Module    (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_CORE_RESOURCE_INDEX
#define BE_CORE_RESOURCE_INDEX

#include "beCore.h"
#include <map>
#include <lean/tags/transitive_ptr.h>
#include <lean/logging/errors.h>

#include <lean/io/numeric.h>

namespace beCore
{

/// Resource index.
template <class Resource, class Info>
class ResourceIndex
{
public:
	/// Value type.
	typedef Resource Resource;
	/// Info type.
	typedef Info Info;

private:
	struct Entry;

	typedef std::map< utf8_string, lean::transitive_ptr<Entry> > string_map;
	
	/// Resource entry.
	struct Entry
	{
		typename string_map::iterator itByName;
		typename string_map::iterator itByFile;

		Info info;

		/// Constructor.
		template <class InfoFW>
		Entry(typename string_map::iterator itByName, typename string_map::iterator itByFile, InfoFW LEAN_FW_REF info)
			: itByName( itByName ),
			itByFile( itByFile ),
			info( LEAN_FORWARD(InfoFW, info) ) { }
#ifndef LEAN0X_NO_RVALUE_REFERENCES
		/// Constructor.
		Entry(Entry &&right)
			: itByName( std::move(right.itByName) ),
			itByFile( std::move(right.itByFile) ),
			info( std::move(right.info) ) { }
#endif
	};

	typedef std::map<const Resource*, Entry> resource_map;
	resource_map m_info;

	string_map m_byName;
	string_map m_byFile;

	template <class Iterator, class Derived>
	class basic_iterator
	{
		friend class ResourceIndex;

	protected:
		Iterator it;

		LEAN_INLINE basic_iterator() { }
		LEAN_INLINE basic_iterator(Iterator it)
			: it(it) { }

	public:
		LEAN_INLINE Derived& operator ++() { ++it; return static_cast<Derived&>(*this); }
		LEAN_INLINE Derived& operator --() { --it; return static_cast<Derived&>(*this); }
		LEAN_INLINE Derived operator ++(int) { Derived prev(it); ++(*this); return prev; }
		LEAN_INLINE Derived operator --(int) { Derived prev(it); --(*this); return prev; }

		template <class It>
		LEAN_INLINE bool operator ==(const It &right) const { return (it == right.it); }
		template <class It>
		LEAN_INLINE bool operator !=(const It &right) const { return (it != right.it); }
	};

	template <class Iterator, class Info>
	class resource_iterator : public basic_iterator< Iterator, resource_iterator<Iterator, Info> >
	{
		friend class ResourceIndex;

	private:
		typedef basic_iterator< Iterator, resource_iterator<Iterator, Info> > base_type;
		
		LEAN_INLINE resource_iterator(Iterator it)
			: base_type(it) { }

	public:
		LEAN_INLINE resource_iterator() { }
		template <class OtherIterator, class OtherInfo>
		LEAN_INLINE resource_iterator(const resource_iterator<OtherIterator, OtherInfo> &right)
			: base_type(right.it) { }

		LEAN_INLINE Info& operator *() const { return it->second.info; }
		LEAN_INLINE Info* operator ->() const { return &it->second.info; }
	};

	enum iterator_tag { name_tag, file_tag };

	template <class Iterator, class Info, iterator_tag Tag>
	class string_iterator : public basic_iterator< Iterator, string_iterator<Iterator, Info, Tag> >
	{
		friend class ResourceIndex;

	private:
		typedef basic_iterator< Iterator, string_iterator<Iterator, Info, Tag> > base_type;
		
		LEAN_INLINE string_iterator(Iterator it)
			: base_type(it) { }

	public:
		LEAN_INLINE string_iterator() { }
		template <class OtherIterator, class OtherInfo>
		LEAN_INLINE string_iterator(const string_iterator<OtherIterator, OtherInfo, Tag> &right)
			: base_type(right.it) { }

		LEAN_INLINE Info& operator *() const { return it->second->info; }
		LEAN_INLINE Info* operator ->() const { return &it->second->info; }

		LEAN_INLINE utf8_ntr key() const { return utf8_ntr(it->first); }
		LEAN_INLINE Info& value() const { return it->second->info; }
	};

public:
	/// Unordered resource iterator type.
	typedef resource_iterator<typename resource_map::iterator, Info> iterator;
	/// Unordered constant resource iterator type.
	typedef resource_iterator<typename resource_map::const_iterator, const Info> const_iterator;
	/// Ordered resource iterator type.
	typedef string_iterator<typename string_map::iterator, Info, name_tag> name_iterator;
	/// Ordered resource iterator type.
	typedef string_iterator<typename string_map::const_iterator, const Info, name_tag> const_name_iterator;
	/// Ordered resource iterator type.
	typedef string_iterator<typename string_map::iterator, Info, file_tag> file_iterator;
	/// Ordered resource iterator type.
	typedef string_iterator<typename string_map::const_iterator, const Info, file_tag> const_file_iterator;

	/// Adds the given resource.
	template <class InfoFW>
	iterator Insert(Resource *resource, const utf8_ntri &name, InfoFW LEAN_FW_REF info)
	{
		typename resource_map::iterator it;

		// Try to insert NEW name link
		typename string_map::iterator itByName = m_byName.insert(
				typename string_map::value_type(name.to<utf8_string>(), nullptr)
			).first;

		// Names must be unique
		if (itByName->second)
			LEAN_THROW_ERROR_CTX("Resource name already taken by another resource", name.c_str());

		try
		{
			// Try to insert NEW resource info block
			// ORDER: Name insertion revertible more easily?
			std::pair<typename resource_map::iterator, bool> insertion = m_info.insert(
					typename resource_map::value_type(
							resource,
							Entry( itByName, m_byFile.end(), LEAN_FORWARD(InfoFW, info) )
						)
				);

			// Do not re-insert resources
			if (!insertion.second)
				// TODO: Actually a programming error? Assert instead of throwing?
				LEAN_THROW_ERROR_CTX("Resource has been inserted before", name.c_str());

			// ORDER: Establish mapping after successful insertion
			it = insertion.first;
			itByName->second = &it->second;
		}
		catch (...)
		{
			// NOTE: Never forget to release name on failure
			if (!itByName->second)
				m_byName.erase(itByName);

			throw;
		}

		return it;
	}

	/// Adds the given name to the given resource.
	name_iterator AddName(iterator where, const utf8_ntri &name)
	{
		// Try to insert NEW name link
		typename string_map::iterator itByName = m_byName.insert(
				typename string_map::value_type(name.to<utf8_string>(), nullptr)
			).first;

		// Ignore redundant calls
		if (itByName->second != &where.it->second)
		{
			// Names must be unique
			if (itByName->second)
				LEAN_THROW_ERROR_CTX("Resource name already taken by another resource", name.c_str());

			// Establish one-way mapping
			itByName->second = &where.it->second;
		}

		return itByName;
	}
	
	/// Changes the name of the given resource.
	name_iterator SetName(iterator where, const utf8_ntri &name, bool bKeepOldName = false)
	{
		typename string_map::iterator itByOldName = where.it->second.itByName;
		typename string_map::iterator itByNewName = AddName(where, name).it;

		// Ignore redundant calls
		if (itByNewName != itByOldName)
		{
			// Establish two-way mapping
			where.it->second.itByName = itByNewName;

			// Release old name
			if (!bKeepOldName)
				m_byName.erase(itByOldName);
		}

		return itByNewName;
	}

	/// Changes the file of the given resource.
	file_iterator SetFile(iterator where, const utf8_ntri &file)
	{
		typename string_map::iterator itByOldFile = where.it->second.itByFile;

		// Try to insert NEW file link
		typename string_map::iterator itByFile = m_byFile.insert(
				typename string_map::value_type(file.to<utf8_string>(), nullptr)
			).first;

		// Ignore redundant calls
		if (itByFile != itByOldFile)
		{
			// Unlink previous resource, if necessary
			if (itByFile->second)
				itByFile->second->itByFile = m_byFile.end();

			// Establish two-way mapping
			itByFile->second = &where.it->second;
			where.it->second.itByFile = itByFile;

			// Release old file
			if (itByOldFile != m_byFile.end())
				m_byFile.erase(itByOldFile);
		}

		return itByFile;
	}

	/// Gets the name of the resource pointed to by the given iterator.
	utf8_ntr GetName(const_iterator where) const { return utf8_ntr(where.it->second.itByName->first); }
	/// Gets the file of the resource pointed to by the given iterator.
	utf8_ntr GetFile(const_iterator where) const
	{
		typename string_map::const_iterator itByFile = where.it->second.itByFile;
		return (itByFile != m_byFile.end()) ? utf8_ntr(itByFile->first) : utf8_ntr("");
	}

	/// Gets a unique name.
	utf8_string GetUniqueName(const utf8_ntri &name) const
	{
		utf8_string unique;

		const size_t nameLength = name.size();
		const size_t maxLength = nameLength + lean::max_int_string_length<uint4>::value + 1;

		// Try unaltered name first
		unique.reserve(maxLength);
		unique = name.to<utf8_string>();
		
		uint4 uniqueIdx = 1;

		// Increment index unto name unique
		while (m_byName.find(unique) != m_byName.end())
		{
			unique.resize(maxLength);
			
			utf8_string::iterator idxBegin = unique.begin() + nameLength;
			
			// Append ".#"
			*idxBegin++ = '.';
			unique.erase(
					lean::int_to_char(idxBegin, uniqueIdx++),
					unique.end()
				);
		}

		return unique;
	}

	/// Gets an iterator to the given resource, if existent.
	iterator Find(const Resource *resource) { return m_info.find(resource); }
	/// Gets an iterator to the given resource, if existent.
	const_iterator Find(const Resource *resource) const { return m_info.find(resource); }
	/// Gets an iterator to the given resource, if existent.
	name_iterator FindByName(const utf8_string &name) { return m_byName.find(name); }
	/// Gets an iterator to the given resource, if existent.
	const_name_iterator FindByName(const utf8_string &name) const { return m_byName.find(name); }
	/// Gets an iterator to the given resource, if existent.
	file_iterator FindByFile(const utf8_string &file) { return m_byFile.find(file); }
	/// Gets an iterator to the given resource, if existent.
	const_file_iterator FindByFile(const utf8_string &file) const { return m_byFile.find(file); }

	/// Gets an iterator to the first resource.
	LEAN_INLINE iterator Begin() { return m_info.begin(); }
	/// Gets an iterator to the first resource.
	LEAN_INLINE const_iterator Begin() const { return m_info.begin(); }
	/// Gets an iterator one past the last resource.
	LEAN_INLINE iterator End() { return m_info.end(); }
	/// Gets an iterator one past the last resource.
	LEAN_INLINE const_iterator End() const { return m_info.end(); }

	/// Gets an iterator to the first resource by name.
	LEAN_INLINE name_iterator BeginByName() { return m_byName.begin(); }
	/// Gets an iterator to the first resource by name.
	LEAN_INLINE const_name_iterator BeginByName() const { return m_byName.begin(); }
	/// Gets an iterator one past the last resource by name.
	LEAN_INLINE name_iterator EndByName() { return m_byName.end(); }
	/// Gets an iterator one past the last resource by name.
	LEAN_INLINE const_name_iterator EndByName() const { return m_byName.end(); }

	/// Gets an iterator to the first resource by file.
	LEAN_INLINE file_iterator BeginByFile() { return m_byFile.begin(); }
	/// Gets an iterator to the first resource by file.
	LEAN_INLINE const_file_iterator BeginByFile() const { return m_byFile.begin(); }
	/// Gets an iterator one past the last resource by file.
	LEAN_INLINE file_iterator EndByFile() { return m_byFile.end(); }
	/// Gets an iterator one past the last resource by file.
	LEAN_INLINE const_file_iterator EndByFile() const { return m_byFile.end(); }

	/// Gets the number of resources.
	LEAN_INLINE uint4 Count() const { return static_cast<uint4>(m_byName.size()); }
};

} // namespace

#endif