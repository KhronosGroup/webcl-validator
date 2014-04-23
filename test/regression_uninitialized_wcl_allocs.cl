// RUN: %webcl-validator "%s" | %opencl-validator

float4 get_pixel(image2d_t img, sampler_t sampler, int2 coords) {
	return read_imagef(img, sampler, coords);
}

__kernel void image_copy(__read_only image2d_t image1, __write_only image2d_t image2)
{
	const int xout = get_global_id(0);
	const int yout = get_global_id(1);
	const sampler_t sampler=CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	float4 pixel;

	pixel = get_pixel(image1, sampler, (int2)(xout,yout));
	write_imagef(image2, (int2)(xout,yout), pixel);
}