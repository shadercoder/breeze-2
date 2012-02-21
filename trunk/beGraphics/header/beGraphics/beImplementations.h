/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_IMPLEMENTATIONS
#define BE_GRAPHICS_IMPLEMENTATIONS

#include <lean/lean.h>

/// @addtogroup GraphicsLibray
/// @{

/// Makes an implementation ID from the given four characters.
#define BE_GRAPHICS_MAKE_IMPLEMENTATION_ID(a, b, c, d)						\
	static_cast<lean::uint4>(														\
		static_cast<lean::uint4>(a) << 3U * lean::size_info<lean::char1>::bits	\
		| static_cast<lean::uint4>(b) << 2U * lean::size_info<lean::char1>::bits	\
		| static_cast<lean::uint4>(c) << 1U * lean::size_info<lean::char1>::bits	\
		| static_cast<lean::uint4>(d) << 0U * lean::size_info<lean::char1>::bits )

namespace beGraphics
{
	/// Implementation ID enumeration.
	enum ImplementationID
	{
		DX11Implementation = BE_GRAPHICS_MAKE_IMPLEMENTATION_ID('D', 'X', '1', '1')	///< DirectX 11 implementation ID.
	};
}

/// DirectX 11 implementation.
#define BE_GRAPHICS_DX11_IMPLEMENTATION DX11Implementation

/// Re-defines the given implementation class template for all implementations.
#define BE_GRAPHICS_DECLARE_IMPLEMENTATIONS(impl, name)	\
	typedef impl<DX11Implementation> name##DX11;
/// Re-defines the given implementation class template for all implementations.
#define BE_GRAPHICS_DECLARE_IMPLEMENTATIONS_1(impl, arg1, name)	\
	typedef impl<arg1, DX11Implementation> name##DX11;
/// Re-defines the given implementation class template for all implementations.
#define BE_GRAPHICS_DECLARE_IMPLEMENTATIONS_2(impl, arg1, arg2, name)	\
	typedef impl<arg1, arg2, DX11Implementation> name##DX11;

/// @}

#endif

#if !defined(BE_GRAPHICS_DIRECTX_11) && (BE_GRAPHICS_IMPLEMENTATION == DX11Implementation || defined(DOXYGEN_READ_THIS))
	/// Defined if DirectX 11 is the active implementation.
	#define BE_GRAPHICS_DIRECTX_11 1
#endif