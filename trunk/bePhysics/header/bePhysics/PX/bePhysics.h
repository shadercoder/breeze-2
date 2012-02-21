/*****************************************************/
/* breeze Engine Physics Module (c) Tobias Zirr 2011 */
/*****************************************************/

#ifndef BE_PHYSICS_PX
#define BE_PHYSICS_PX

#include "../bePhysics.h"
#include <lean/meta/strip.h>
#include <lean/meta/type_traits.h>

/// @addtogroup GlobalSwitches
/// Use this to export classes/methods/functions as part of the public library API.
#define BE_PHYSICS_PX_API BE_PHYSICS_API

/// @addtogroup PhysicsLibrayPX bePhysics PhysX implementation
/// For a better overview, see <a href="namespaces.html">Namespaces</a>.
/// @see <a href="namespaces.html">Namespaces</a>
/// @{

/// Main namespace of the bePhysics library.
namespace bePhysics
{
	/// Checks whether the given object belongs to the PhyxX implementation.
	LEAN_INLINE bool IsPX(const Implementation *pImpl)
	{
		return Is(pImpl, PXImplementation);
	}

	/// Converts the given abstract type to its PhysX implementation type, if available.
	template <class Abstract>
	struct ToImplementationPX;

	namespace Impl
	{
		LEAN_DEFINE_HAS_TYPE(Type);

		template <bool, class>
		struct RobustToImplementationPXImpl { };

		template <class Abstract>
		struct RobustToImplementationPXImpl<true, Abstract>
		{
			typedef typename lean::strip_modifiers<Abstract>::template undo<
					typename ToImplementationPX<
						typename lean::strip_modifiers<Abstract>::type
					>::Type
				>::type Type;
		};

		template <class Abstract>
		struct RobustToImplementationPX
			: public RobustToImplementationPXImpl<
				has_type_Type< ToImplementationPX<typename lean::strip_modifiers<Abstract>::type> >::value,
				Abstract > { };

	} // namespace

	/// Casts the given abstract type to its PhysX implementation type, if available.
	template <class Abstract>
	LEAN_INLINE typename Impl::RobustToImplementationPX<Abstract>::Type* ToImpl(Abstract *pAbstract)
	{
		LEAN_ASSERT(pAbstract->GetImplementationID() == PXImplementation);
		return static_cast< typename Impl::RobustToImplementationPX<Abstract>::Type* >(pAbstract);
	}
	/// Casts the given abstract type to its PhysX implementation type, if available.
	template <class Abstract>
	LEAN_INLINE typename Impl::RobustToImplementationPX<Abstract>::Type& ToImpl(Abstract &abstr4ct)
	{
		return *ToImpl( lean::addressof(abstr4ct) );
	}

} // namespace

/// @}

#endif