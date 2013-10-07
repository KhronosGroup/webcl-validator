// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

typedef event_t wcl_event_t;

__kernel void json_event(
// CHECK: error: Can't determine host type for event_t.
    event_t event,
// CHECK: error: Can't determine host type for event_t.
    wcl_event_t event_typedef,
// CHECK: error: The read_write image access qualifier is not supported.
    __read_write image2d_t image)
{
}
