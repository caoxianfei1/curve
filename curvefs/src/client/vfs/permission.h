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

enum ModeMask {
    WANT_READ = 0b100,
    WANT_WRITE = 0b010,
    WANT_EXEC = 0b001,
};

class Permission {
 public:
    explicit Permission(PermissionOption option);

    bool Check(Ino ino, ModeMask mmask, InodeAttr* attr);

 private:
    uint32_t accessMode(InodeAttr *attr, uint32_t uid, const std::vector<uint32_t>& gids);
    
 private:
    PermissionOption option_;
};

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

#endif  // CURVEFS_SRC_CLIENT_VFS_PERMISSION_H_
