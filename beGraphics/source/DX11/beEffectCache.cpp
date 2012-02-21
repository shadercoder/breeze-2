/*****************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*****************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/DX11/beEffectCache.h"
#include "beGraphics/DX11/beEffect.h"
#include "beGraphics/DX11/beDevice.h"
#include "beGraphics/DX/beEffect.h"
#include "beGraphics/DX/beIncludeManager.h"

#include <lean/smart/com_ptr.h>
#include <lean/smart/cloneable_obj.h>
#include <unordered_map>
#include <vector>
#include <lean/containers/dynamic_array.h>

#include <beCore/beFileWatch.h>
#include <beCore/beDependenciesImpl.h>

#include <lean/io/raw_file.h>
#include <lean/io/mapped_file.h>
#include <lean/io/filesystem.h>

#include <lean/logging/errors.h>
#include <lean/logging/log.h>

namespace beGraphics
{

namespace DX11
{

/// Effect cache implementation
struct EffectCache::M
{
	lean::cloneable_obj<beCore::PathResolver> resolver;
	lean::cloneable_obj<beCore::ContentProvider> provider;

	lean::com_ptr<ID3D11Device> pDevice;
	utf8_string cacheDir;

	/// Effect
	struct ObservedEffect : public beCore::FileObserver
	{
		lean::resource_ptr<Effect> pEffect;
		const utf8_string *mangledFile;

		EffectCache *pCache;
		utf8_string file;
		utf8_string unresolvedFile;
		typedef lean::dynamic_array<char> macro_backing_store;
		macro_backing_store macroStore;
		typedef lean::dynamic_array<D3D_SHADER_MACRO> macro_vector;
		macro_vector macros;

		beCore::Dependency<beGraphics::Effect*>* dependency;

		/// Constructor.
		ObservedEffect(lean::resource_ptr<Effect> pEffect, EffectCache *pCache)
			: pEffect(pEffect.transfer()),
			mangledFile(),
			pCache(pCache),
			dependency() { }

		/// Method called whenever an observed effect has changed.
		void FileChanged(const lean::utf8_ntri &file, lean::uint8 revision);
	};

	typedef std::unordered_map<utf8_string, ObservedEffect> effect_map;
	effect_map effects;

	typedef std::unordered_map<ID3DX11Effect*, ObservedEffect*> effect_info_map;
	effect_info_map effectInfo;

	beCore::FileWatch fileWatch;

	beCore::DependenciesImpl<beGraphics::Effect*> dependencies;

	/// Constructor.
	M(ID3D11Device *pDevice, const utf8_ntri &cacheDir, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
		: pDevice(pDevice),
		cacheDir( lean::canonical_path<utf8_string>(cacheDir) ),
		resolver(resolver),
		provider(contentProvider)
	{
		LEAN_ASSERT(pDevice != nullptr);
	}
};

namespace
{

/// Appends all files passed to the store vector.
struct VectorIncludeTracker : public DX::IncludeTracker
{
	typedef std::vector<utf8_string> file_vector;
	file_vector *Files;	///< Files.

	/// Constructor.
	VectorIncludeTracker(file_vector *files)
		: Files(files) { }

	/// Called for each actual include file, passing the resolved file & revision.
	void Track(const utf8_ntri &file)
	{
		if (Files)
			Files->push_back(file.to<utf8_string>());
	}
};

/// Gets the name of the corresponding cache file.
utf8_string GetCacheFile(const EffectCache::M &m, const lean::utf8_ntri &file)
{
	return lean::append_path<utf8_string>(
		m.cacheDir,
		lean::get_filename<utf8_string>(file).append(".fxc") );
}

/// Gets the name of the corresponding cache dependency file.
utf8_string GetDependencyFile(const lean::utf8_ntri &cacheFile)
{
	return cacheFile.c_str() + utf8_string(".deps");
}

/// Gets the revision of all dependencies stored in the given dependency file.
uint8 GetDependencyRevision(EffectCache::M &m, const lean::utf8_ntri &dependencyFile, std::vector<utf8_string> *pIncludeFiles)
{
	uint8 dependencyRevision = 0;

	try
	{
		lean::com_ptr<beCore::Content> pDependencyData = m.provider->GetContent(dependencyFile);

		const utf8_t *dependencies = reinterpret_cast<const utf8_t*>( pDependencyData->Bytes() );
		const utf8_t *dependenciesBase = dependencies;
		const utf8_t *dependenciesEnd = dependencies + pDependencyData->Size() / sizeof(utf8_t);

		while (dependencies != dependenciesEnd)
		{
			// Read up to end or next zero delimiter
			while (dependencies != dependenciesEnd && *dependencies)
				++dependencies;

			// Ignore empty strings
			if (dependencies != dependenciesBase)
			{
				// NOTE: might not be null-terminated => construct string
				beCore::Exchange::utf8_string path = m.resolver->Resolve( utf8_string(dependenciesBase, dependencies), false );

				// Check if dependency still existent
				if (!path.empty())
				{
					dependencyRevision = max( dependencyRevision, m.provider->GetRevision(path) );

					if (pIncludeFiles)
						pIncludeFiles->push_back( utf8_string(path.begin(), path.end()) );
				}
			}

			// Move on to next dependency string
			if (dependencies != dependenciesEnd)
				dependenciesBase = ++dependencies;
		}
	}
	catch (...)
	{
		LEAN_LOG_ERROR_CTX("Could not open cached effect dependency file", dependencyFile.c_str());
	}

	return dependencyRevision;
}

// Compiles and caches the given effect.
lean::com_ptr<ID3DBlob, true> CompileAndCacheEffect(EffectCache::M &m, const lean::utf8_ntri &file, const D3D_SHADER_MACRO *pMacros,
		const lean::utf8_ntri &cacheFile, const lean::utf8_ntri &dependencyFile,
		const lean::utf8_ntri &unresolvedFile, std::vector<utf8_string> *pIncludeFiles)
{
	typedef std::vector<utf8_string> file_vector;
	file_vector rawDependencies;

	// Track includes & raw dependencies
	VectorIncludeTracker includeTracker(pIncludeFiles);
	VectorIncludeTracker rawDependencyTracker(&rawDependencies);
	DX::IncludeManager includeManager(*m.resolver, *m.provider, &includeTracker, &rawDependencyTracker);

	// Compile effect
	lean::com_ptr<ID3DBlob> pData = CompileEffect(file, pMacros, &includeManager);

	// Replace resolved main file
	if (!rawDependencies.empty())
		rawDependencies[0] = unresolvedFile.to<utf8_string>();

	try
	{
		{
			// Cache compiled effect
			lean::mapped_file mappedFile(cacheFile, pData->GetBufferSize(), true, lean::file::overwrite, lean::file::sequential);
			memcpy(mappedFile.data(), pData->GetBufferPointer(), pData->GetBufferSize());
		}

		try
		{
			// Dump dependencies
			lean::raw_file rawDependencyFile(dependencyFile, lean::file::write, lean::file::overwrite, lean::file::sequential);

			for (file_vector::const_iterator it = rawDependencies.begin(); it != rawDependencies.end(); ++it)
				// NOTE: Include zero delimiters
				rawDependencyFile.write(it->c_str(), it->size() + 1);
		}
		catch (...)
		{
			LEAN_LOG_ERROR_CTX(
				"Failed to dump dependencies effect to cache",
				dependencyFile.c_str() );
		}
	}
	catch (...)
	{
		LEAN_LOG_ERROR_CTX(
			"Failed to write compiled effect to cache",
			cacheFile.c_str() );
	}

	return pData.transfer();
}

// Re-compiles the given effect.
lean::com_ptr<ID3DX11Effect, true> RecompileEffect(EffectCache::M &m, const lean::utf8_ntri &file, const D3D_SHADER_MACRO *pMacros,
		const lean::utf8_ntri &mangledFile, const lean::utf8_ntri &unresolvedFile, std::vector<utf8_string> *pIncludeFiles)
{
	utf8_string cacheFile = GetCacheFile(m, mangledFile);
	utf8_string dependencyFile = GetDependencyFile(cacheFile);

	if (pIncludeFiles)
		pIncludeFiles->clear();

	lean::com_ptr<ID3DBlob> pCompiledData = CompileAndCacheEffect(m, file, pMacros, cacheFile, dependencyFile, unresolvedFile, pIncludeFiles);
	return CreateEffect(pCompiledData, m.pDevice);
}

// Compiles or loads the given effect.
lean::com_ptr<ID3DX11Effect, true> CompileOrLoadEffect(EffectCache::M &m, const lean::utf8_ntri &file, const D3D_SHADER_MACRO *pMacros,
		const lean::utf8_ntri &mangledFile, const lean::utf8_ntri &unresolvedFile, std::vector<utf8_string> *pIncludeFiles)
{
	lean::com_ptr<ID3DX11Effect> pEffect;

	utf8_string cacheFile = GetCacheFile(m, mangledFile);
	utf8_string dependencyFile = GetDependencyFile(cacheFile);

	uint8 fileRevision = m.provider->GetRevision(file);
	uint8 cacheRevision = m.provider->GetRevision(cacheFile);

	// Also walk cache dependencies to detect all relevant changes
	if (cacheRevision >= fileRevision)
		fileRevision = max( fileRevision, GetDependencyRevision(m, dependencyFile, pIncludeFiles) );

	// WARNING: Keep alive until effect has been created
	{
		lean::com_ptr<beCore::Content> pCachedData;
		lean::com_ptr<ID3DBlob> pCompiledData;

		const char *pEffectData = nullptr;
		uint4 effectDataSize = 0;

		// Load from cache, if up-to-date, ...
		if (cacheRevision >= fileRevision)
		{
			try
			{
				pCachedData = m.provider->GetContent(cacheFile);
				pEffectData = pCachedData->Bytes();
				effectDataSize = static_cast<uint4>( pCachedData->Size() );
			}
			catch (...)
			{
				LEAN_LOG_ERROR_CTX("Error while trying to load cached effect", cacheFile.c_str());
			}
		}

		// ... recompile otherwise
		if (!pEffectData)
		{
			pCompiledData = CompileAndCacheEffect(m, file, pMacros, cacheFile, dependencyFile, unresolvedFile, pIncludeFiles);
			pEffectData = static_cast<const char*>( pCompiledData->GetBufferPointer() );
			effectDataSize = static_cast<uint4>(pCompiledData->GetBufferSize());
		}

		pEffect = CreateEffect(pEffectData, effectDataSize, m.pDevice);
	}

	return pEffect.transfer();
}

} // namespace

// Constructor.
EffectCache::EffectCache(ID3D11Device *pDevice, const utf8_ntri &cacheDir, const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
	: m( new M(pDevice, cacheDir, resolver, contentProvider) )
{
}

// Destructor.
EffectCache::~EffectCache()
{
}

namespace
{

/// Gets the mangled file name.
inline utf8_string GetMangledFilename(const lean::utf8_ntri &path, const EffectMacro *pMacros, size_t macroCount)
{
	utf8_string mangledFile;
	
	// Decorate path
	if (pMacros && macroCount > 0)
	{
		beCore::Exchange::utf8_string mangled = MangleFilename(path, pMacros, macroCount);
		mangledFile.assign(mangled.begin(), mangled.end());
	}
	else
		mangledFile.assign(path.begin(), path.end());

	return mangledFile;
}

/// Gets the mangled file name.
inline utf8_string GetMangledFilename(const lean::utf8_ntri &path, const utf8_ntri &macros)
{
	utf8_string mangledFile;
	
	// Decorate path
	if (!macros.empty())
	{
		beCore::Exchange::utf8_string mangled = MangleFilename(path, macros);
		mangledFile.assign(mangled.begin(), mangled.end());
	}
	else
		mangledFile.assign(path.begin(), path.end());

	return mangledFile;
}

/// Adds the given effect.
inline EffectCache::M::effect_map::iterator AddEffect(EffectCache::M &m, const utf8_ntri &unresolvedFile, const utf8_ntri &path, const utf8_string &mangledFile,
	EffectCache::M::ObservedEffect::macro_vector &macros, EffectCache::M::ObservedEffect::macro_backing_store &macroStore, EffectCache *pCache)
{
	EffectCache::M::effect_map::iterator itEffect;

	typedef std::vector<utf8_string> file_vector;
	file_vector includeFiles;

	// WARNING: Resource pointer invalid after transfer
	{
		LEAN_LOG("Attempting to load effect \"" << mangledFile << "\"");

		lean::resource_ptr<Effect> pEffect = lean::bind_resource(
				new Effect( 
					CompileOrLoadEffect(m, path, &macros[0], mangledFile, unresolvedFile, &includeFiles).get(),
					pCache
				)
			);

		LEAN_LOG("Effect \"" << unresolvedFile.c_str() << "\" created successfully");

		// Insert effect into cache
		itEffect = m.effects.insert(
				EffectCache::M::effect_map::value_type(
					mangledFile,
					EffectCache::M::ObservedEffect( pEffect.transfer(), pCache )
				)
			).first;
		itEffect->second.mangledFile = &itEffect->first;
		itEffect->second.file.assign(path.begin(), path.end());
		itEffect->second.unresolvedFile.assign(unresolvedFile.begin(), unresolvedFile.end());
		itEffect->second.macroStore = LEAN_MOVE(macroStore);
		itEffect->second.macros = LEAN_MOVE(macros);
	}

	// Link back to effect info
	m.effectInfo[*itEffect->second.pEffect] = &itEffect->second;

	// Allow for monitoring of dependencies
	itEffect->second.dependency = m.dependencies.AddDependency(itEffect->second.pEffect);

	// Watch entire include graph
	for (file_vector::const_iterator itFile = includeFiles.begin(); itFile != includeFiles.end(); ++itFile)
		m.fileWatch.AddObserver(*itFile, &itEffect->second);

	return itEffect;
}

} // namespace

// Gets the given effect compiled using the given options from file.
Effect* EffectCache::GetEffect(const lean::utf8_ntri &unresolvedFile, const EffectMacro *pMacros, size_t macroCount)
{
	// Get absolute path
	beCore::Exchange::utf8_string path = m->resolver->Resolve(unresolvedFile, true);

	// Try to find cached effect
	utf8_string mangledFile = GetMangledFilename(path, pMacros, macroCount);
	M::effect_map::iterator itEffect = m->effects.find(mangledFile);

	if (itEffect == m->effects.end())
	{
		M::ObservedEffect::macro_backing_store macroStore;
		EffectCache::M::ObservedEffect::macro_vector macros = DX::ToAPI( pMacros, (pMacros) ? macroCount : 0, macroStore );

		itEffect = AddEffect(
				*m, unresolvedFile, path, mangledFile,
				macros, macroStore,
				this
			);
	}

	return itEffect->second.pEffect;
}

// Gets the given effect compiled using the given options from file.
Effect* EffectCache::GetEffect(const lean::utf8_ntri &unresolvedFile, const utf8_ntri &macroString)
{
	// Get absolute path
	beCore::Exchange::utf8_string path = m->resolver->Resolve(unresolvedFile, true);
	
	// Try to find cached effect
	utf8_string mangledFile = GetMangledFilename(path, macroString);
	M::effect_map::iterator itEffect = m->effects.find(mangledFile);

	if (itEffect == m->effects.end())
	{
		M::ObservedEffect::macro_backing_store macroStore;
		EffectCache::M::ObservedEffect::macro_vector macros = DX::ToAPI( macroString, macroStore );

		itEffect = AddEffect(
				*m, unresolvedFile, path, mangledFile,
				macros, macroStore,
				this
			);
	}

	return itEffect->second.pEffect;
}

// Gets the given effect compiled using the given options from file, if it has been loaded.
Effect* EffectCache::IdentifyEffect(const lean::utf8_ntri &file, const utf8_ntri &macros) const
{
	// Get absolute path
	beCore::Exchange::utf8_string path = m->resolver->Resolve(file, true);
	
	// Try to find cached effect
	utf8_string mangledFile = GetMangledFilename(path, macros);
	M::effect_map::const_iterator itEffect = m->effects.find(mangledFile);

	return (itEffect != m->effects.end())
		? itEffect->second.pEffect
		: nullptr;
}

// Notifies dependent listeners about dependency changes.
void EffectCache::NotifyDependents()
{
	m->dependencies.NotifiyAllSyncDependents();
}

// Gets the file (or name) of the given effect.
utf8_ntr EffectCache::GetFile(const beGraphics::Effect &effect, beCore::Exchange::utf8_string *pMacros, bool *pIsFile) const
{
	utf8_ntr file("");

	M::effect_info_map::const_iterator itInfo = m->effectInfo.find( ToImpl(effect) );

	if (itInfo != m->effectInfo.end())
	{
		file = utf8_ntr(itInfo->second->file);

		if (pMacros)
		{
			pMacros->reserve( itInfo->second->macroStore.size() );
			DX::ToString(&itInfo->second->macros.front(), &itInfo->second->macros.back() + 1, *pMacros);
		}

		if (pIsFile)
			// Back pointer only valid for files
			*pIsFile = (itInfo->second->pCache != nullptr);
	}

	return file;
}

// Gets the dependencies registered for the given effect.
beCore::Dependency<beGraphics::Effect*>* EffectCache::GetDependency(const beGraphics::Effect &effect)
{
	M::effect_info_map::iterator itEffect = m->effectInfo.find( ToImpl(effect) );

	return (itEffect != m->effectInfo.end())
		? itEffect->second->dependency
		: nullptr;
}

// Method called whenever an observed effect has changed.
void EffectCache::M::ObservedEffect::FileChanged(const lean::utf8_ntri &file, lean::uint8 revision)
{
	typedef std::vector<utf8_string> file_vector;
	file_vector includeFiles;

	// MONITOR: Non-atomic asynchronous assignment!
	this->pEffect = lean::bind_resource(
			new Effect( 
				RecompileEffect(*this->pCache->m, file, &this->macros[0], *this->mangledFile, this->unresolvedFile, &includeFiles).get(),
				this->pCache
			)
		);

	// Enhance watched include graph
	for (file_vector::const_iterator itFile = includeFiles.begin(); itFile != includeFiles.end(); ++itFile)
		this->pCache->m->fileWatch.AddOrKeepObserver(*itFile, this);

	// Notify dependent listeners
	this->pCache->m->dependencies.DependencyChanged(this->dependency, this->pEffect);
}

} // namespace

// Creates a new effect cache.
lean::resource_ptr<EffectCache, true> CreateEffectCache(const Device &device, const utf8_ntri &cacheDir, 
	const beCore::PathResolver &resolver, const beCore::ContentProvider &contentProvider)
{
	return lean::bind_resource(
		new DX11::EffectCache(
				ToImpl(device),
				cacheDir,
				resolver,
				contentProvider
			)
		);
}

} // namespace