// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void
LotsOfMathBuiltins(
    __global uchar4 *result,
    const float4 mu,
    const float4 diffuse,
    const float epsilon)
{
    int tx = get_global_id(0);
    int ty = get_global_id(1);
    int sx = get_global_size(0);
    int sy = get_global_size(1);
    int index = ty * 100 + tx;

    float4 coord = (float4)((float)tx, (float)ty, 0.0f, 0.0f);

    float nx = fast_length(diffuse) - fast_length(coord);
    float ny = fast_length(mu) - fast_length(coord);
    float nz = fast_length(diffuse) - fast_length(mu);

    float3 normal = fast_normalize((float3)( nx, ny, nz ));

    float3 light_dir = fast_normalize( normal - normal );
    float3 eye_dir = fast_normalize( normal - normal );
    float NdotL = dot( normal, light_dir );
    float3 reflect_dir = light_dir - 2.0f * NdotL * normal;

    reflect_dir += fabs(normal) * 0.5f;
// CHECK-NOT: error: can't convert between vector values of different size ('float3' and 'int')
    float3 diff = reflect_dir * fmax(NdotL, 0.0f);
    float3 specular = NdotL * half_powr( fmax( dot(eye_dir, reflect_dir), 0.0f), nz );

    float fB = 2.0f * dot( diff, specular );
    float fB2 = fB * fB;
    float fC = dot( diffuse, diffuse ) - fB2;
    float fT = (fB2 - 4.0f * fC);
    float fD = half_sqrt( fT );
    float fT0 = ( -fB + fD ) * 0.5f;
    float fT1 = ( -fB - fD ) * 0.5f;
    fT = fmin(fT0, fT1);

    float4 color = (float4)(fD, fT0, fT1, fT);
    uchar4 output = convert_uchar4_sat_rte(color * 255.0f);
    result[index] = output;
}
