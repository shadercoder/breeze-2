/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#pragma once
#ifndef BE_GRAPHICS_DEVICE_CONTEXT_DX11
#define BE_GRAPHICS_DEVICE_CONTEXT_DX11

#include "beGraphics.h"
#include "../beDeviceContext.h"
#include <beCore/beWrapper.h>
#include <lean/smart/com_ptr.h>
#include <D3D11.h>

namespace beGraphics
{

namespace DX11
{

/// Clears all pixel shader output resources.
BE_GRAPHICS_DX11_API void UnbindAllRenderTargets(ID3D11DeviceContext *context);
/// Clears all compute shader output resources.
BE_GRAPHICS_DX11_API void UnbindAllComputeTargets(ID3D11DeviceContext *context);
/// Clears all output resources.
BE_GRAPHICS_DX11_API void UnbindAllTargets(ID3D11DeviceContext *context);

/// Clears all shader resources.
BE_GRAPHICS_DX11_API void UnbindAllShaderResources(ID3D11DeviceContext *context);

/// Device context implementation.
class DeviceContext : public beCore::IntransitiveWrapper<ID3D11DeviceContext, DeviceContext>,
	public beGraphics::DeviceContext
{
private:
	lean::com_ptr<ID3D11DeviceContext> m_pContext;
	
public:
	/// Constructor.
	LEAN_INLINE DeviceContext(ID3D11DeviceContext *pContext)
		: m_pContext( LEAN_ASSERT_NOT_NULL(pContext) ) { }
	
	/// Gets the D3D device context.
	LEAN_INLINE ID3D11DeviceContext*const& GetInterface() const { return m_pContext.get(); }
	/// Gets the D3D device context.
	LEAN_INLINE ID3D11DeviceContext*const& GetContext() const { return m_pContext.get(); }

	/// Clears all state.
	BE_GRAPHICS_DX11_API void ClearState() LEAN_OVERRIDE { m_pContext->ClearState(); };

	/// Gets the implementation identifier.
	LEAN_INLINE ImplementationID GetImplementationID() const { return DX11Implementation; };
};

template <> struct ToImplementationDX11<beGraphics::DeviceContext> { typedef DeviceContext Type; };

} // namespace

} // namespace

#endif