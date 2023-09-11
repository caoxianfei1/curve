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

#ifndef CURVEFS_SRC_CLIENT_VFS_PERMISSION_H_
#define CURVEFS_SRC_CLIENT_VFS_PERMISSION_H_

#include <string>

#include "curvefs/src/client/common/config.h"

namespace curvefs {
namespace client {
namespace vfs {

using ::curvefs::client::common::PermissionOption;

class Permission {
 public:
    const

    enum {
        NONE = 0,
        WANT_EXEC = 1,
        WANT_WRITE = 2,
        WANT_READ = 4,
    };

 public:
    explicit Permission(PermissionOption option);

    uint16_t GetMode(uint16_t type, uint16_t mode);

    CURVEFS_ERROR Check(const InodeAttr& attr, uins32_t want);

 private:
    bool IsSuperUser();

    bool GidInGroup(uint16_t gid);

    uint16_t GetFilePermission(const InodeAttr& attr);

 private:
    PermissionOption option_;
};

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

#endif  // CURVEFS_SRC_CLIENT_VFS_PERMISSION_H_
