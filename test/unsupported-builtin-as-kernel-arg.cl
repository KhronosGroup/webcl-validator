// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

typedef event_t wcl_event_t;

__kernel void event_kernel(
// CHECK: error: 'event_t' cannot be used as the type of a kernel parameter
    event_t event,
// CHECK: error: 'wcl_event_t' (aka 'event_t') cannot be used as the type of a kernel parameter
    wcl_event_t event_typedef)
{
}
