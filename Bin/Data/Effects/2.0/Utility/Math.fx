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
	return max2( max(values.xy, values.zw); );
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

#endif