// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

void baz()
{ 
    int a = 0;
    // CHECK: error: initializing 'sampler_t' with an expression of incompatible type 'int'
    sampler_t sampler = a;
}

__kernel void sampler_type_errors2(sampler_t sampler1)
{
    sampler_t sampler2 = CLK_ADDRESS_CLAMP;
    // CHECK: error: used type 'sampler_t' where arithmetic or pointer type is required
    sampler_t sampler3 = sampler2 ? 0 : 0;
    // CHECK: error: incompatible operand types ('sampler_t' and 'int')
    sampler_t sampler4 = 1 ? sampler2 : 0;
    // CHECK: error: invalid operands to binary expression ('sampler_t' and 'int')
    sampler_t sampler5 = sampler1 | 0;
}

