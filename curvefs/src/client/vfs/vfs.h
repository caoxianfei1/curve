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
 * Created Date: 2023-06-29
 * Author: Jingli Chen (Wine93)
 */

#ifndef CURVEFS_SRC_CLIENT_VFS_VFS_H_
#define CURVEFS_SRC_CLIENT_VFS_VFS_H_

#include <string>
#include <memory>

#include "src/common/configuration.h"
#include "curvefs/src/client/common/config.h"
#include "curvefs/src/client/vfs/meta.h"
#include "curvefs/src/client/vfs/cache.h"
#include "curvefs/src/client/vfs/config.h"
#include "curvefs/src/client/vfs/handlers.h"
#include "curvefs/src/client/vfs/permission.h"
#include "curvefs/src/client/vfs/operations.h"

namespace curvefs {
namespace client {
namespace vfs {

using ::curve::common::Configuration;
using ::curvefs::client::common::VFSOption;

class VFS {
 public:
    VFS() = default;

    int SetPermission(uint32_t uid, const std::vector<uint32_t>& gids, uint16_t umask, bool needCheck);

    // NOTE: |cfg| include all configures for client.conf
    int Mount(const std::string& fsname,
              const std::string& mountpoint,
              std::shared_ptr<Configure> cfg);

    int Umount();

    // directory interface
    int MkDir(const std::string& path, int mode);

    int OpenDir(const std::string& path, DirStream* stream);

    ssize_t ReadDir(DirStream* stream, DirEntry* dirEntry);

    int CloseDir(DirStream* stream);

    int RmDir(const std::string& path);

    // file interface
    int64_t Open(const std::string& path, int flags, int mode);

    ssize_t Read(int fd, char* buffer, size_t count);

    ssize_t Write(int fd, char* buffer, size_t count);

    int FSync(int fd);

    int Close(int fd);

    int Unlink(const std::string& path);

    // attr & xattr & others
    int Stat(const std::string& path, struct stat* stat);

    int FStat(int fd, struct stat* stat);

    int Chown(const std::string &path, uint32_t uid, uint32_t gid);

 private:
    Configuration Convert(std::shared_ptr<Configure> cfg);

    CURVEFS_ERROR Lookup(const std::string& pathname,
                         bool followSymlink,
                         Entry* entry);

 private:
    VFSOption option_;
    std::shared_ptr<Operations> op_;  // filesystem operations
    std::shared_ptr<Permission> permission_;
    std::shared_ptr<FileHandlers> handlers_;
    std::shared_ptr<EntryCache> entryCache_;
    std::shared_ptr<AttrCache> attrCache_;
    uint32_t uid_;
    std::vector<uint32_t> gids_;
};

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

#endif  // CURVEFS_SRC_CLIENT_VFS_VFS_H_
