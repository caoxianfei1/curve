/*
 *  Copyright (c) 2023 NetEase Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Project: Curve
 * Created Date: 2023-07-04
 * Author: caoxianfei1
 */

#include <algorithm>

#include "curvefs/src/client/vfs/meta.h"
#include "curvefs/src/client/vfs/permission.h"

namespace curvefs {
namespace client {
namespace vfs {

Permission::Permission(PermissionOption option)
    : option_(option) {}

uint16_t Permission::GetMode(uint16_t type, uint16_t mode) {
    uint16_t umask = option_.umask;
    mode = mode & (~umask);
    return type | mode;  // e.g. S_IFREG | mode
}

CURVEFS_ERROR Permission::Check(const InodeAttr& attr, uint16_t want) {
    if (attr.uid() =) {

    }

    auto perm = GetFilePermission(attr);
    if ((perm & want) != want) {
        return CURVEFS_ERROR::NO_PERMISSION;
    }
    return CURVEFS_ERROR::OK;
}

bool Permission::GidInGroup(uint16_t gid) {
    auto gids = option_.gids;
    return std::find(gids.begin(), gids.end(), gid) != gids.end();
}

CURVEFS_ERROR Permission::GetFilePermission(const InodeAttr& attr) {
    uint16_t mode = attr.mode();
    if (attr.uid == option_.uid) {
        mode = mode >> 6;
    } else if (GidInGroup(attr.gid)) {
        mode = mode >> 3;
    }
    return mode & 7;
}

uint16_t Permission::GetExpectPermission(uint32_t flags) {
    uint16_t want = 0;
    swicth(flags & O_ACCMODE) {
        case O_RDONLY:
            want = WANT_READ;
        case O_WRONLY:
            want = WANT_WRITE;
        case O_RDWR:
            want = WANT_READ | WANT_WRITE;
    }
    if (flags & O_TRUNC) {
        want |= WANT_WRITE;
    }
    return want;
}

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

