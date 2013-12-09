// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

typedef event_t wcl_event_t;

__kernel void event_kernel(
// CHECK: error: Unsupported builtin type event_t used as a kernel parameter.
    event_t event,
// CHECK: error: Unsupported builtin type event_t used as a kernel parameter.
    wcl_event_t event_typedef)
{
}
