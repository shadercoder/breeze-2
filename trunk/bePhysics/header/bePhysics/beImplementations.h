/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_IMPLEMENTATIONS
#define BE_PHYSICS_IMPLEMENTATIONS

#include <lean/lean.h>

/// @addtogroup PhysicsLibray
/// @{

/// Makes an implementation ID from the given four characters.
#define BE_PHYSICS_MAKE_IMPLEMENTATION_ID(a, b, c, d)						\
	static_cast<lean::uint4>(														\
		static_cast<lean::uint4>(a) << 3U * lean::size_info<lean::char1>::bits	\
		| static_cast<lean::uint4>(b) << 2U * lean::size_info<lean::char1>::bits	\
		| static_cast<lean::uint4>(c) << 1U * lean::size_info<lean::char1>::bits	\
		| static_cast<lean::uint4>(d) << 0U * lean::size_info<lean::char1>::bits )

namespace bePhysics
{
	/// Implementation ID enumeration.
	enum ImplementationID
	{
		PXImplementation = BE_PHYSICS_MAKE_IMPLEMENTATION_ID('P', 'X', '3', '+')	///< PhysX 3+ implementation ID.
	};
}

/// PhysX 3+ implementation.
#define BE_PHYSICS_PX_IMPLEMENTATION PXImplementation

/// @}

#endif

#if !defined(BE_PHYSICS_PHYSX) && (BE_PHYSICS_IMPLEMENTATION == PXImplementation || defined(DOXYGEN_READ_THIS))
	/// Defined if PhysX 3+ is the active implementation.
	#define BE_PHYSICS_PHYSX 1
#endif