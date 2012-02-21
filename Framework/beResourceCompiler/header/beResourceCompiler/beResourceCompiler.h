/****************************************************************/
/* breeze Framework Resource Compiler Lib  (c) Tobias Zirr 2011 */
/****************************************************************/

#ifndef BE_RESOURCECOMPILER
#define BE_RESOURCECOMPILER

/// @addtogroup GlobalSwitches Global switches used for configuration
/// @see GlobalMacros
/// @see AssortedSwitches
/// @{

#ifdef DOXYGEN_READ_THIS
	/// Define this when compiling this library into a static library.
	#define BE_RESOURCECOMPILER_DONT_EXPORT
	#undef BE_RESOURCECOMPILER_DONT_EXPORT
#endif

/// @}

/// @defgroup AssortedSwitches Assorted Switches
/// @see GlobalSwitches

/// @addtogroup GlobalMacros Global macros
/// @see GlobalSwitches
/// @see CPP0X
/// @{

#ifndef BE_RESOURCECOMPILER_DONT_EXPORT
	
	#ifdef _MSC_VER

		#ifdef BERESOURCECOMPILER_EXPORTS
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_RESOURCECOMPILER_API __declspec(dllexport)
		#else
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_RESOURCECOMPILER_API __declspec(dllimport)
		#endif

	#else
		// TODO: Implement for GCC?
		#error Exporting not yet implemented for this compiler
	#endif

#else
	/// Use this to export classes/methods/functions as part of the public library API.
	#define BE_RESOURCECOMPILER_API
#endif

/// @}

#include <beCore/beCore.h>

/// @addtogroup ResourceCompilerLibrary Resource compiler framework library
/// For a better overview, see <a href="namespaces.html">Namespaces</a>.
/// @see <a href="namespaces.html">Namespaces</a>
/// @{

/// Main namespace of the beResourceCompiler library.
namespace beResourceCompiler
{
	using namespace lean::types;
	using namespace lean::strings::types;

	/// Opens a message box containing version information.
	BE_RESOURCECOMPILER_API void InfoBox();
}

/// @}

#endif