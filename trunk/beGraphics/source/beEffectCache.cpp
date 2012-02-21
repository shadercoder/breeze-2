/*******************************************************/
/* breeze Engine Graphics Module  (c) Tobias Zirr 2011 */
/*******************************************************/

#include "beGraphicsInternal/stdafx.h"
#include "beGraphics/beEffectCache.h"

#include "beGraphics/DX/beEffect.h"

namespace beGraphics
{

// Mangles the given file name & macros.
Exchange::utf8_string MangleFilename(const lean::utf8_ntri &file, const EffectMacro *pMacros, size_t macroCount)
{
	Exchange::utf8_string mangledFile;

	if (pMacros && macroCount)
	{
		size_t mangledLength = file.size() + 2;

		for (size_t i = 0; i < macroCount; ++i)
			mangledLength += 2 + pMacros->Name.size() + pMacros->Definition.size();
		
		mangledFile.reserve(mangledLength);
		
		mangledFile.append(file.begin(), file.end());
		mangledFile.append(1, '(');

		for (size_t i = 0; i < macroCount; ++i)
			if (!pMacros->Name.empty())
			{
				mangledFile.append(1, '-')
					.append(pMacros->Name.begin(), pMacros->Name.end());

				if (!pMacros->Definition.empty())
					mangledFile.append(1, '=')
						.append(pMacros->Definition.begin(), pMacros->Definition.end());
			}

		mangledFile.append(1, ')');
	}
	else
		mangledFile.assign(file.begin(), file.end());

	return mangledFile;
}

// Mangles the given file name & macros.
Exchange::utf8_string MangleFilename(const lean::utf8_ntri &file, const lean::utf8_ntri &macros)
{
	Exchange::utf8_string mangledFile;

	if (!macros.empty())
	{
		mangledFile.reserve(file.size() + macros.size() + 3);

		mangledFile.append(file.begin(), file.end());
		mangledFile.append(1, '(');

		const char *itMacro = macros.c_str();

		while (*itMacro)
		{
			lean::range<const char*> macroName;
			lean::range<const char*> macroDefinition;

			itMacro = DX::ExtractNextMacro(itMacro, macroName, macroDefinition);

			if (!macroName.empty())
			{
				mangledFile.append(1, '-')
					.append(macroName.begin(), macroName.end());

				if (!macroDefinition.empty())
					mangledFile.append(1, '=')
						.append(macroDefinition.begin(), macroDefinition.end());
			}
		}

		mangledFile.append(1, ')');
	}
	else
		mangledFile.assign(file.begin(), file.end());

	return mangledFile;
}

} // namespace