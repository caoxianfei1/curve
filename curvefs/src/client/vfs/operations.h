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
 * Created Date: 2023-07-07
 * Author: Jingli Chen (Wine93)
 */

#include <fcntl.h>  /* O_CREAT */

#include <string>
#include <memory>

#include "curvefs/src/client/fuse_client.h"
#include "curvefs/src/client/filesystem/error.h"
#include "curvefs/src/client/filesystem/dir_cache.h"
#include "curvefs/src/client/vfs/meta.h"

#ifndef CURVEFS_SRC_CLIENT_VFS_OPERATIONS_H_
#define CURVEFS_SRC_CLIENT_VFS_OPERATIONS_H_

namespace curvefs {
namespace client {
namespace vfs {

using ::curvefs::client::FuseClient;
using ::curvefs::client::filesystem::FileSystem;
using ::curvefs::client::filesystem::DirEntryList;
using ::curvefs::client::filesystem::CURVEFS_ERROR;
using ::curvefs::client::common::PermissionOption;


class Operations {
 public:
    // init
    virtual CURVEFS_ERROR Umount() = 0;

    // directory*
    virtual CURVEFS_ERROR MkDir(Ino parent,
                                const std::string& name,
                                uint16_t mode) = 0;

    virtual CURVEFS_ERROR OpenDir(Ino ino, uint64_t* fh) = 0;

    virtual CURVEFS_ERROR ReadDir(Ino ino,
                                  uint64_t fh,
                                  std::shared_ptr<DirEntryList>* entries) = 0;

    virtual CURVEFS_ERROR CloseDir(Ino ino) = 0;

    virtual CURVEFS_ERROR RmDir(Ino parent, const std::string& name) = 0;

    // file*
    virtual CURVEFS_ERROR Create(Ino parent,
                                 const std::string& name,
                                 uint16_t mode) = 0;

    virtual CURVEFS_ERROR Open(Ino ino, uint32_t flags) = 0;

    virtual CURVEFS_ERROR Read(Ino ino,
                               uint64_t offset,
                               char* buffer,
                               size_t size,
                               size_t* nread) = 0;

    virtual CURVEFS_ERROR Write(Ino ino,
                                uint64_t offset,
                                char* buffer,
                                size_t size,
                                size_t* nwritten) = 0;

    virtual CURVEFS_ERROR Flush(Ino ino) = 0;

    virtual CURVEFS_ERROR Close(Ino ino) = 0;

    virtual CURVEFS_ERROR Unlink(Ino parent, const std::string& name) = 0;

    // others
    virtual CURVEFS_ERROR StatFS(Ino ino, struct statvfs* statvfs) = 0;

    virtual CURVEFS_ERROR Lookup(Ino parent,
                                 const std::string& name,
                                 EntryOut* entryOut) = 0;

    virtual CURVEFS_ERROR GetAttr(Ino ino, AttrOut* attrOut) = 0;

    virtual CURVEFS_ERROR SetAttr(Ino ino, struct stat* stat, int toSet) = 0;

    virtual CURVEFS_ERROR ReadLink(Ino ino, std::string* link) = 0;

    virtual CURVEFS_ERROR Rename(Ino parent,
                                 const std::string& name,
                                 Ino newparent,
                                 const std::string& newname) = 0;

    // utility
    virtual void Attr2Stat(InodeAttr* attr, struct stat* stat) = 0;

    virtual void SetPermissionOption(PermissionOption option) = 0;
};

class OperationsImpl : public Operations {
 public:
    explicit OperationsImpl(std::shared_ptr<FuseClient> client);

    // init
    CURVEFS_ERROR Umount() override;

    // directory*
    CURVEFS_ERROR MkDir(Ino parent,
                        const std::string& name,
                        uint16_t mode) override;

    CURVEFS_ERROR OpenDir(Ino ino, uint64_t* fh) override;

    CURVEFS_ERROR ReadDir(Ino ino,
                          uint64_t fh,
                          std::shared_ptr<DirEntryList>* entries) override;

    CURVEFS_ERROR CloseDir(Ino ino) override;

    CURVEFS_ERROR RmDir(Ino parent, const std::string& name) override;

    // file*
    CURVEFS_ERROR Create(Ino parent,
                         const std::string& name,
                         uint16_t mode) override;

    CURVEFS_ERROR Open(Ino ino, uint32_t flags) override;

    CURVEFS_ERROR Read(Ino ino,
                       uint64_t offset,
                       char* buffer,
                       size_t size,
                       size_t* nread) override;

    CURVEFS_ERROR Write(Ino ino,
                        uint64_t offset,
                        char* buffer,
                        size_t size,
                        size_t* nwritten) override;

    CURVEFS_ERROR Flush(Ino ino) override;

    CURVEFS_ERROR Close(Ino ino) override;

    CURVEFS_ERROR Unlink(Ino parent, const std::string& name) override;

    // others
    CURVEFS_ERROR StatFS(Ino ino, struct statvfs* statvfs) override;

    CURVEFS_ERROR Lookup(Ino parent,
                         const std::string& name,
                         EntryOut* entryOut) override;

    CURVEFS_ERROR GetAttr(Ino ino, AttrOut* attrOut) override;

    CURVEFS_ERROR SetAttr(Ino ino, struct stat* stat, int toSet) override;

    CURVEFS_ERROR ReadLink(Ino ino, std::string* link) override;

    CURVEFS_ERROR Rename(Ino parent,
                         const std::string& name,
                         Ino newparent,
                         const std::string& newname) override;

    // utility
    void Attr2Stat(InodeAttr* attr, struct stat* stat) override;

    void SetPermissionOption(PermissionOption option) override;

 private:
    std::shared_ptr<FuseClient> client_;
    std::shared_ptr<FileSystem> fs_;
    PermissionOption psOption_;
};

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

#endif  // CURVEFS_SRC_CLIENT_VFS_OPERATIONS_H_
