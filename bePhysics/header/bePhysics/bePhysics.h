/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#pragma once
#ifndef BE_PHYSICS
#define BE_PHYSICS

/// @addtogroup GlobalSwitches Global switches used for configuration
/// @see GlobalMacros
/// @see AssortedSwitches
/// @{

#ifdef DOXYGEN_READ_THIS
	/// Define this when not compiling this library into a DLL.
	#define BE_PHYSICS_NO_EXPORT
	#undef BE_PHYSICS_NO_EXPORT
#endif

/// @}

/// @defgroup AssortedSwitches Assorted Switches
/// @see GlobalSwitches

/// @addtogroup GlobalMacros Global macros
/// @see GlobalSwitches
/// @see CPP0X
/// @{

#ifndef BE_PHYSICS_NO_EXPORT
	
	#ifdef _MSC_VER

		#ifdef BEPHYSICS_EXPORTS
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_PHYSICS_API __declspec(dllexport)
		#else
			/// Use this to export classes/methods/functions as part of the public library API.
			#define BE_PHYSICS_API __declspec(dllimport)
		#endif

	#else
		// TODO: Implement for GCC?
		#error Exporting not yet implemented for this compiler
	#endif

#else
	/// Use this to export classes/methods/functions as part of the public library API.
	#define BE_PHYSICS_API
#endif

/// @}

#include <beCore/beCore.h>

/// @addtogroup PhysicsLibray bePhysics library
/// For a better overview, see <a href="namespaces.html">Namespaces</a>.
/// @see <a href="namespaces.html">Namespaces</a>
/// @{

#ifdef DOXYGEN_READ_THIS
	/// Define this to select the PhysX implementation.
	#define BE_PHYSICS_PHYSX_3
	#undef BE_PHYSICS_PHYSX_3
#endif

// Default to PhysX if nothing else specified
#if !defined(BE_PHYSICS_PHYSX_3) && 1 // ... // TODO: Update for every new alternative
	#define BE_PHYSICS_PHYSX_3
#endif

/// Main namespace of the bePhysics library.
namespace bePhysics
{
	// Import important types
	using namespace lean::types;
	LEAN_REIMPORT_NUMERIC_TYPES;
	using namespace lean::strings::types;

	/// Implementation ID enumeration.
	enum ImplementationID
	{
		PX3Implementation = LEAN_MAKE_WORD_4('P', 'X', '3', '+')	///< PhysX 3+ implementation ID.
	};

	#ifdef BE_PHYSICS_PHYSX_3
		/// Active implementation ID.
		static const ImplementationID ActiveImplementation = PX3Implementation;
	#endif

	/// Physics implementation interface.
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
	BE_PHYSICS_API void InfoBox();
}

/// Shorthand namespace.
namespace breeze
{
#ifndef DOXYGEN_READ_THIS
	/// bePhysics namespace alias.
	namespace bepx = ::bePhysics;
#else
	/// bePhysics namespace alias.
	namespace bepx { using namespace ::bePhysics; }
#endif
}

/// @}

#endif