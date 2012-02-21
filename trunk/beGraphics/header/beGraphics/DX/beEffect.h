/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_EFFECT_DX
#define BE_GRAPHICS_EFFECT_DX

#include "beGraphics.h"
#include "../beEffectCache.h"
#include <D3DCommon.h>
#include <lean/containers/dynamic_array.h>

namespace beGraphics
{

namespace DX
{

/// Converts the given effect macros into a string representation.
template <class String>
LEAN_INLINE void ToString(const D3D_SHADER_MACRO *macros, const D3D_SHADER_MACRO *macrosEnd, String &string)
{
	for (; macros != macrosEnd; ++macros)
	{
		if (macros->Name)
		{
			string.append(macros->Name);

			if (macros->Definition)
			{
				string.append(1, '=')
					.append(macros->Definition);
			}

			string.append(1, ';');
		}
	}
}

/// Extracts the next macro.
template <class Char>
LEAN_INLINE const Char* ExtractNextMacro(const Char* it, lean::range<const Char*> &name, lean::range<const Char*> &definition)
{
	// Ignore redundant separators
	while (*it == ';' || *it == ' ')
		++it;

	name.begin() = it;

	// Find OPTIONAL definition
	while (*it != '=' && *it != ';' && *it)
		++it;

	name.end() = it;

	// Check for optional definition
	if (*it == '=')
	{
		definition.begin() = ++it;

		// Find definition end
		while (*it != ';' && *it)
			++it;

		definition.end() = it;
	}

	return it;
}

/// Converts the given string representation into an array of effect macros.
template <class String>
LEAN_INLINE lean::dynamic_array<D3D_SHADER_MACRO> ToAPI(const String &string, lean::dynamic_array<char> &backingStore)
{
	lean::dynamic_array<D3D_SHADER_MACRO> macrosDX;

	size_t macroCount = 0;
	size_t storeLength = 0;
	bool bReadSeparator = true;

	for (const char *it = string.c_str(); *it; ++it)
	{
		if (*it == ';' || bReadSeparator && *it == ' ')
			bReadSeparator = true;
		else
		{
			// Ignores redundant separators
			if (bReadSeparator)
			{
				++macroCount;
				bReadSeparator = false;
			}

			++storeLength;
		}
	}

	// Add space for terminating zeroes of names
	// NOTE: Space for terminating zeroes of definitions included (replacing '=')
	storeLength += macroCount;

	backingStore.reset(storeLength);
	macrosDX.reset(macroCount + 1);

	const char *itMacro = string.c_str();

	for (size_t i = 0; i < macroCount; ++i)
	{
		lean::range<const char*> macroName;
		lean::range<const char*> macroDefinition;

		itMacro = ExtractNextMacro(itMacro, macroName, macroDefinition);

		size_t nameLength = macroName.size();
		size_t definitonLength = macroDefinition.size();
		
		D3D_SHADER_MACRO &macroDX = macrosDX.push_back();

		macroDX.Name = backingStore.push_back_n(nameLength + 1);
		macroDX.Definition = backingStore.push_back_n(definitonLength + 1);

		memcpy( const_cast<char*>(macroDX.Name), macroName.begin(), nameLength );
		memcpy( const_cast<char*>(macroDX.Definition), macroDefinition.begin(), definitonLength );
		
		const_cast<char*>(macroDX.Name)[nameLength] = 0;
		const_cast<char*>(macroDX.Definition)[definitonLength] = 0;
	}

	D3D_SHADER_MACRO &endOfMacrosDX = macrosDX.push_back();
	endOfMacrosDX.Name = nullptr;
	endOfMacrosDX.Definition = nullptr;

	return macrosDX;
}

/// Converts the given effect macros to DirectX shader macros.
LEAN_INLINE lean::dynamic_array<D3D_SHADER_MACRO> ToAPI(const EffectMacro *macros, size_t macroCount, lean::dynamic_array<char> &backingStore)
{
	lean::dynamic_array<D3D_SHADER_MACRO> macrosDX(macroCount + 1);

	size_t storeLength = 0;
	
	for (size_t i = 0; i < macroCount; ++i)
	{
		const EffectMacro &macro = macros[i];
		storeLength += macro.Name.size() + 1;
		storeLength += macro.Definition.size() + 1;
	}

	backingStore.reset(storeLength);

	for (size_t i = 0; i < macroCount; ++i)
	{
		const EffectMacro &macro = macros[i];
		D3D_SHADER_MACRO &macroDX = macrosDX.push_back();

		macroDX.Name = backingStore.push_back_n(macro.Name.size());
		macroDX.Definition = backingStore.push_back_n(macro.Definition.size());

		memcpy( const_cast<char*>(macroDX.Name), macro.Name.c_str(), macro.Name.size() + 1 );
		memcpy( const_cast<char*>(macroDX.Definition), macro.Definition.c_str(), macro.Definition.size() + 1 );
	}

	D3D_SHADER_MACRO &endOfMacrosDX = macrosDX.push_back();
	endOfMacrosDX.Name = nullptr;
	endOfMacrosDX.Definition = nullptr;

	return macrosDX;
}

/// Converts the given effect macros to DirectX shader macros.
LEAN_INLINE lean::dynamic_array<D3D_SHADER_MACRO> ToAPI(const EffectMacro *macros, size_t macroCount)
{
	lean::dynamic_array<D3D_SHADER_MACRO> macrosDX(macroCount + 1);
	
	for (size_t i = 0; i != macroCount; ++i)
	{
		const EffectMacro &macro = macros[i];
		D3D_SHADER_MACRO &macroDX = macrosDX.push_back();

		macroDX.Name = macro.Name.c_str();
		macroDX.Definition = macro.Definition.c_str();
	}

	D3D_SHADER_MACRO &endOfMacrosDX = macrosDX.push_back();
	endOfMacrosDX.Name = nullptr;
	endOfMacrosDX.Definition = nullptr;

	return macrosDX;
}

} // namespace

using DX::ToAPI;

} // namespace

#endif