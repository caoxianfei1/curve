libcurvefs
===

SDK C/C++ library for CurveFS.

Example
===

C
---

```c
#include "libcurvefs.h"

int instance_ptr = curvefs_create();
int rc = curvefs_mount(instance_ptr);
if (rc != 0) {
    // mount failed
}

rc = curvefs_mkdir(instance_ptr, "/mydir")
if (rc != 0) {
    // mkdir failed
}
```

C++
---

```cpp
auto mount = std::make_shared<CurveFSMount>();
auto rc = mount->Mount();
if (rc != 0) {
    // mount failed
}
rc = mount->Mkdir("/mydir");
if (rc != 0) {
    // mkdir failed
}
```
