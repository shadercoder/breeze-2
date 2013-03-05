/****************************************************/
/* breeze Engine Assets Module (c) Tobias Zirr 2013 */
/****************************************************/

#pragma once
#ifndef BE_ASSETS
#define BE_ASSETS

/// @addtogroup GlobalSwitches Global switches used for configuration
/// @see GlobalMacros
/// @see AssortedSwitches
/// @{

#ifdef DOXYGEN_READ_THIS
	/// Define this when not compiling this library into a DLL.
	#define BE_ASSETS_NO_EXPORT
	#undef BE_ASSETS_NO_EXPORT
#endif

/// @}

/// @defgroup AssortedSwitches Assorted Switches
/// @see GlobalSwitches

/// @addtogroup GlobalMacros Global macros
/// @see GlobalSwitches
/// @see CPP0X
/// @{

#ifndef BE_ASSETS_NO_EXPORT
	
	#ifdef _MSC_VER

		#ifdef BEASSETS_EXPORTS
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_ASSETS_API __declspec(dllexport)
		#else
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_ASSETS_API __declspec(dllimport)
		#endif

	#else
		// TODO: Implement for GCC?
		#error Exporting not yet implemented for this compiler
	#endif

#else
	/// Use this to export classes/methods/functions as part of the public library API.
	#define BE_ASSETS_API
#endif

/// @}

#include <beCore/beCore.h>

/// @addtogroup AssetLibray beAssets library
/// For a better overview, see <a href="namespaces.html">Namespaces</a>.
/// @see <a href="namespaces.html">Namespaces</a>
/// @{

/// Main namespace of the beAssets library.
namespace beAssets
{
	// Import important types
	using namespace lean::types;
	LEAN_REIMPORT_NUMERIC_TYPES;
	using namespace lean::strings::types;

	/// NOP, may be used to enforce module linkage.
	BE_ASSETS_API void Link();
	/// Opens a message box containing version information.
	BE_ASSETS_API void InfoBox();
}

/// @}

#endif