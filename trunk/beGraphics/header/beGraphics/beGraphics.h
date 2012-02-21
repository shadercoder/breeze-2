/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS
#define BE_GRAPHICS

/// @addtogroup GlobalSwitches Global switches used for configuration
/// @see GlobalMacros
/// @see AssortedSwitches
/// @{

#ifdef DOXYGEN_READ_THIS
	/// Define this when not compiling this library into a DLL.
	#define BE_GRAPHICS_NO_EXPORT
	#undef BE_GRAPHICS_NO_EXPORT
#endif

/// @}

/// @defgroup AssortedSwitches Assorted Switches
/// @see GlobalSwitches

/// @addtogroup GlobalMacros Global macros
/// @see GlobalSwitches
/// @{

#ifndef BE_GRAPHICS_NO_EXPORT
	
	#ifdef _MSC_VER

		#ifdef BEGRAPHICS_EXPORTS
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_GRAPHICS_API __declspec(dllexport)
		#else
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_GRAPHICS_API __declspec(dllimport)
		#endif

	#else
		// TODO: Implement for GCC?
		#error Exporting not yet implemented for this compiler
	#endif

#else
	/// Use this to export classes/methods/functions as part of the public library API.
	#define BE_GRAPHICS_API
#endif

/// @}

#include <beCore/beCore.h>
#include "beImplementations.h"

/// @addtogroup GraphicsLibray beGraphics library
/// For a better overview, see <a href="namespaces.html">Namespaces</a>.
/// @see <a href="namespaces.html">Namespaces</a>
/// @{

#ifndef BE_GRAPHICS_IMPLEMENTATION
	/// Define this to select a graphics implementation.
	#define BE_GRAPHICS_IMPLEMENTATION BE_GRAPHICS_DX11_IMPLEMENTATION
#endif

// Define implementation switches
#include "beImplementations.h"

/// Main namespace of the beGraphics library.
namespace beGraphics
{
	// Import important types
	using namespace lean::types;
	using namespace lean::strings::types;

	/// Active implementation ID.
	static const ImplementationID ActiveImplementation = BE_GRAPHICS_IMPLEMENTATION;

	/// Graphics implementation interface.
	class Implementation
	{
	public:
		virtual ~Implementation() { }

		/// Gets the implementation identifier.
		virtual ImplementationID GetImplementationID() const = 0;
	};

	/// Checks whether the given object belongs to the given implementation.
	LEAN_INLINE bool Is(const Implementation *pImpl, ImplementationID implID)
	{
		return (pImpl->GetImplementationID() == implID);
	}

	/// Opens a message box containing version information.
	BE_GRAPHICS_API void InfoBox();
}

/// Shorthand namespace.
namespace breeze
{
	/// Graphics namespace alias.
	namespace beg = ::beGraphics;
}

/// @}

#endif