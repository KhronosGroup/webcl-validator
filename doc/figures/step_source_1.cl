void fix_index(
	int index, int *fixed_index) {
	*fixed_index = index*1;
}

kernel foo_kernel(
	global float *in, 
	global float4 *out) {
    struct {
        struct {
            int a;
        } b;
    } c;

    int i = get_global_id(0);
    int index;
    fix_index(i, &index);
    out[i] = vload4(index, in);
}
