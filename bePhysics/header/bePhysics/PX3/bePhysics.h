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
#define BE_PHYSICS_PX3_API BE_PHYSICS_API
#define BE_PHYSICS_PX_API BE_PHYSICS_PX3_API

namespace bePhysics
{

/// @addtogroup PhysicsLibrayPX3 bePhysics PhysX 3 implementation
/// For a better overview, see <a href="namespaces.html">Namespaces</a>.
/// @see <a href="namespaces.html">Namespaces</a>
/// @{

/// Main namespace of the PhysX 3 implementation.
namespace PX3
{
	/// Checks whether the given object belongs to the PhysX implementation.
	LEAN_INLINE bool IsPX3(const Implementation *pImpl)
	{
		return Is(pImpl, PX3Implementation);
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
		LEAN_ASSERT(!pAbstract || pAbstract->GetImplementationID() == PX3Implementation);
		return static_cast< typename Impl::RobustToImplementationPX<Abstract>::Type* >(pAbstract);
	}
	/// Casts the given abstract type to its PhysX implementation type, if available.
	template <class Abstract>
	LEAN_INLINE typename Impl::RobustToImplementationPX<Abstract>::Type& ToImpl(Abstract &abstr4ct)
	{
		return *ToImpl( lean::addressof(abstr4ct) );
	}

} // namespace

using PX3::ToImpl;

} // namespace

/// @}

/// Shorthand namespace.
namespace breeze
{
	/// Physics namespace alias.
	namespace bepx3 = ::bePhysics::PX3;
}

#endif