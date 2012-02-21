/******************************************************/
/* breeze Engine Graphics Module (c) Tobias Zirr 2011 */
/******************************************************/

#ifndef BE_GRAPHICS_D3D11
#define BE_GRAPHICS_D3D11

#include "beGraphics.h"
#include <D3D11.h>

namespace beGraphics
{

namespace DX11
{

/// DirectX 11 types redefined.
namespace API
{

/// Direct3D 11 device.
typedef ID3D11Device Device;
/// Direct3D 11 device context.
typedef ID3D11DeviceContext DeviceContext;

/// Direct3D 11 buffer.
typedef ID3D11Resource Resource;
/// Direct3D 11 buffer.
typedef ID3D11Buffer Buffer;
/// Direct3D 11 buffer.
typedef ID3D11Texture1D Texture1D;
/// Direct3D 11 buffer.
typedef ID3D11Texture2D Texture2D;
/// Direct3D 11 buffer.
typedef ID3D11Texture3D Texture3D;

/// Direct3D 11 input layout.
typedef ID3D11InputLayout InputLayout;

/// Direct3D 11 sampler state.
typedef ID3D11SamplerState SamplerState;
/// Direct3D 11 rasterizer state.
typedef ID3D11RasterizerState RasterizerState;
/// Direct3D 11 depth-stencil state.
typedef ID3D11DepthStencilState DepthStencilState;
/// Direct3D 11 blend state.
typedef ID3D11BlendState BlendState;

/// Direct3D 11 counter.
typedef ID3D11Counter Counter;
/// Direct3D 11 query.
typedef ID3D11Query Query;
/// Direct3D 11 predicate.
typedef ID3D11Predicate Predicate;

} // namespace

//using namespace API;

} // namespace

} // namespace

#endif