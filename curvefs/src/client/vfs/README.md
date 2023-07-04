
Usage Example
===

```C++
#include "curvefs/src/client/vfs/vfs.h"

auto vfs = std::make_shared<VFS>();
auto cfg = Configure::Default();
cfg->Set("mdsOpt.rpcRetryOpt.addrs", "10.0.0.0:6700,10.0.0.1:6701,10.0.0.2:6702");
cfg->Set("fs.accessLogging", "true");

auto rc = vfs->Mount("myfs", "/", cfg);
if (rc != 0) {
    // handle error
}

rc = vfs->MkDir("/dir1", 0644);
if (rc != 0) {
    // handle error
}
```
