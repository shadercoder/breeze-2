#ifndef BE_UTILITY_MATH_H
#define BE_UTILITY_MATH_H

/// Computes the minimum component of the given vector.
float min2(float2 values)
{
	return min(values.x, values.y);
}
/// Computes the maximum component of the given vector.
float max2(float2 values)
{
	return max(values.x, values.y);
}

/// Computes the minimum component of the given vector.
float min3(float3 values)
{
	return min( min2(values.xy), values.z );
}
/// Computes the maximum component of the given vector.
float max3(float3 values)
{
	return max( max2(values.xy), values.z );
}

/// Computes the minimum component of the given vector.
float min4(float4 values)
{
	return min2( min(values.xy, values.zw) );
}
/// Computes the maximum component of the given vector.
float max4(float4 values)
{
	return max2( max(values.xy, values.zw) );
}


/// Computes the minimum component of the given vector.
uint min2(uint2 values)
{
	return min(values.x, values.y);
}
/// Computes the maximum component of the given vector.
uint max2(uint2 values)
{
	return max(values.x, values.y);
}

/// Computes the minimum component of the given vector.
uint min3(uint3 values)
{
	return min( min2(values.xy), values.z );
}
/// Computes the maximum component of the given vector.
uint max3(uint3 values)
{
	return max( max2(values.xy), values.z );
}

/// Computes the minimum component of the given vector.
uint min4(uint4 values)
{
	return min2( min(values.xy, values.zw) );
}
/// Computes the maximum component of the given vector.
uint max4(uint4 values)
{
	return max2( max(values.xy, values.zw) );
}

/// Computes the median of the given vector.
float median(float3 values)
{
	float m = dot(values, 1.0f);
	m -= min3(values);
	m -= max3(values);
	return m;
}

/// Computes the median of the given vector.
float median(float4 values)
{
	float m = dot(values, 1.0f);
	m -= min4(values);
	m -= max4(values);
	return m * 0.5f;
}

/// Interpolates the given set of three values.
float interpolate(float v0, float v1, float v2, float3 b)
{
	return b.x * v0 + b.y * v1 + b.z * v2;
}

/// Interpolates the given set of three vectors.
float2 interpolate(float2 v0, float2 v1, float2 v2, float3 b)
{
	return b.x * v0 + b.y * v1 + b.z * v2;
}

/// Interpolates the given set of three vectors.
float3 interpolate(float3 v0, float3 v1, float3 v2, float3 b)
{
	return b.x * v0 + b.y * v1 + b.z * v2;
}

/// Interpolates the given set of three vectors.
float4 interpolate(float4 v0, float4 v1, float4 v2, float3 b)
{
	return b.x * v0 + b.y * v1 + b.z * v2;
}

/// Returns the given value mod 3.
int mod3(int i)
{
	return i % 3;
}

/// Returns the given value mod 3.
uint mod3(uint i)
{
	return i % 3u;
}

/// Either 1 or -1.
float sign1(float x)
{
	return (0.0f <= x) ? 1.0f : -1.0f;
}
float2 sign1(float2 x)
{
	return float2(sign1(x.x), sign1(x.y));
}
float3 sign1(float3 x)
{
	return float3(sign1(x.x), sign1(x.y), sign1(x.z));
}
float4 sign1(float4 x)
{
	return float4(sign1(x.x), sign1(x.y), sign1(x.z), sign1(x.w));
}

/// Square.
float sq(float x)
{
	return x * x;
}
float2 sq(float2 x)
{
	return x * x;
}
float3 sq(float3 x)
{
	return x * x;
}
float4 sq(float4 x)
{
	return x * x;
}

float lengthsq(float x)
{
	return x * x;
}
float lengthsq(float2 x)
{
	return dot(x, x);
}
float lengthsq(float3 x)
{
	return dot(x, x);
}
float lengthsq(float4 x)
{
	return dot(x, x);
}

/// sqrt(1 - x^2).
float pyt1(float x)
{
	return sqrt( saturate(1.0f - x * x) );
}

/// ceil(x / d).
uint ceil_div(uint x, uint d)
{
	return (x + d - 1u) / d;
}
uint2 ceil_div(uint2 x, uint2 d)
{
	return (x + d - 1u) / d;
}
uint3 ceil_div(uint3 x, uint3 d)
{
	return (x + d - 1u) / d;
}
uint4 ceil_div(uint4 x, uint4 d)
{
	return (x + d - 1u) / d;
}

/// Replaces NaNs.
float repnan(float v, float r)
{
	return isnan(v) ? r : v;
}
float2 repnan(float2 v, float2 r)
{
	return float2( repnan(v.x, r.x), repnan(v.y, r.y) );
}
float3 repnan(float3 v, float3 r)
{
	return float3( repnan(v.x, r.x), repnan(v.y, r.y), repnan(v.z, r.z) );
}
float4 repnan(float4 v, float4 r)
{
	return float4( repnan(v.x, r.x), repnan(v.y, r.y), repnan(v.z, r.z), repnan(v.w, r.w) );
}

/// Replaces according to the given mask.
float repmask(bool b, float v, float r)
{
	return b ? r : v;
}
float2 repmask(bool2 b, float2 v, float2 r)
{
	return float2( repmask(b.x, v.x, r.x), repmask(b.y, v.y, r.y) );
}
float3 repmask(bool3 b, float3 v, float3 r)
{
	return float3( repmask(b.x, v.x, r.x), repmask(b.y, v.y, r.y), repmask(b.z, v.z, r.z) );
}
float4 repmask(bool4 b, float4 v, float4 r)
{
	return float4( repmask(b.x, v.x, r.x), repmask(b.y, v.y, r.y), repmask(b.z, v.z, r.z), repmask(b.w, v.w, r.w) );
}

static const float PI = 3.141592653589793f;

#endif