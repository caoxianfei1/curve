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

#include "absl/strings/string_view.h"
#include "absl/strings/str_split.h"

#include <string>
#include <utility>
#include <vector>

#include "curvefs/src/client/helper.h"
#include "curvefs/src/client/logger/access_log.h"
#include "curvefs/src/client/filesystem/utils.h"
#include "curvefs/src/client/filesystem/error.h"
#include "curvefs/src/client/vfs/config.h"
#include "curvefs/src/client/vfs/meta.h"
#include "curvefs/src/client/vfs/utils.h"
#include "curvefs/src/client/vfs/vfs.h"

namespace curvefs {
namespace client {
namespace vfs {

using ::curvefs::client::Helper;
using ::curvefs::client::logger::AccessLogGuard;
using ::curvefs::client::logger::StrFormat;
using ::curvefs::client::filesystem::IsSymlink;
using ::curvefs::client::filesystem::StrErr;
using ::curvefs::client::filesystem::StrMode;

Configuration VFS::Convert(std::shared_ptr<Configure> cfg) {
    Configuration config;
    cfg->Iterate([&](const std::string& key, const std::string& value){
        config.SetStringValue(key, value);
    });
    return config;
}

int VFS::Mount(const std::string& fsname,
               const std::string& mountpoint,
               std::shared_ptr<Configure> cfg) {
    auto helper = Helper();
    auto config = Convert(cfg);
    std::shared_ptr<FuseClient> client;
    auto yes = helper.NewClientForSDK(fsname, mountpoint, &config, &client);
    if (!yes) {
        return SysErr(CURVEFS_ERROR::INTERNAL);
    }

    op_ = std::make_shared<OperationsImpl>(client);
    return SysErr(CURVEFS_ERROR::OK);
}

int VFS::Umount() {
    return SysErr(CURVEFS_ERROR::OK);
}

int VFS::MkDir(const std::string& path, int mode) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&]() {
        return StrFormat("mkdir (%s): %s", path, StrErr(rc));
    });

    Entry parent;
    rc = Lookup(filepath::ParentDir(path), true, &parent);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    rc = op_->MkDir(parent.ino, filepath::Filename(path), mode);
    return SysErr(rc);
}

int VFS::RmDir(const std::string& path) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("rmdir (%s): %s", path, StrErr(rc));
    });

    Entry parent;
    rc = Lookup(filepath::ParentDir(path), true, &parent);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    rc = op_->RmDir(parent.ino, filepath::Filename(path));
    return SysErr(rc);
}

int VFS::OpenDir(const std::string& path, DirStream* stream) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("opendir (%s): %s", path, StrErr(rc));
    });

    Entry entry;
    rc = Lookup(path, true, &entry);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    stream->ino = entry.ino;
    stream->offset = 0;
    rc = op_->OpenDir(entry.ino, &stream->fh);
    return SysErr(rc);
}

ssize_t VFS::ReadDir(DirStream* stream, DirEntry* dirEntry) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("readdir (%d): %s", stream->fh, StrErr(rc));
    });

    std::shared_ptr<DirEntryList> entries;
    rc = op_->ReadDir(stream->ino, stream->fh, &entries);
    if (rc == CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    // FIXME(Wine93): readdir once
    if (stream->offset >= entries->Size()) {
        return 0;
    }

    entries->At(stream->offset, dirEntry);
    stream->offset++;
    return 1;
}

int VFS::CloseDir(DirStream* stream) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("closedir (%d): %s", stream->fh, StrErr(rc));
    });

    rc = op_->CloseDir(stream->ino);
    return SysErr(rc);
}

int64_t VFS::Open(const std::string& path, int flags, int mode) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("open (%d:0%04o): %s",
                         path, StrMode(mode), StrErr(rc));
    });

    Entry entry;
    rc = Lookup(path, true, &entry);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    // TODO(Wine93): O_CREAT
    //if (flags & O_CREAT) {
    //    rc = op_->Create()
    //}

    rc = op_->Open(entry.ino, flags);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    // TODO(Wine93): O_APPEND
    // FIXME(Wine93): int64_t -> int
    return handlers_->NextHandler();
}

int VFS::Close(int fd) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("close (%d): %s", fd, StrErr(rc));
    });

    std::shared_ptr<FileHandler> fh;
    bool yes = handlers_->GetHandler(fd, &fh);
    if (!yes) {  // already closed or never opened
        rc = CURVEFS_ERROR::BAD_FD;
        return SysErr(rc);
    }

    rc = op_->Close(fh->ino);
    if (rc == CURVEFS_ERROR::OK) {
        handlers_->FreeHandler(fd);
    }
    return SysErr(rc);
}

int VFS::Unlink(const std::string& path) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("unlink (%s): %s", path, StrErr(rc));
    });

    Entry parent;
    rc = Lookup(filepath::ParentDir(path), true, &parent);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    rc = op_->Unlink(parent.ino, filepath::Filename(path));
    return SysErr(rc);
}

ssize_t VFS::Read(int fd, char* buffer, size_t count) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("read (%d, %zu): %s", fd, count, StrErr(rc));
    });

    std::shared_ptr<FileHandler> fh;
    bool yes = handlers_->GetHandler(fd, &fh);
    if (!yes) {  // already closed or never opened
        rc = CURVEFS_ERROR::BAD_FD;
        return SysErr(rc);
    }

    size_t nread;
    rc = op_->Write(fh->ino, fh->offset, buffer, count, &nread);
    if (rc == CURVEFS_ERROR::OK) {
        fh->offset += nread;
    }
    return SysErr(rc);
}

ssize_t VFS::Write(int fd, char* buffer, size_t count) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("write (%d, %zu): %s", fd, count, StrErr(rc));
    });

    std::shared_ptr<FileHandler> fh;
    bool yes = handlers_->GetHandler(fd, &fh);
    if (!yes) {  // already closed or never opened
        rc = CURVEFS_ERROR::BAD_FD;
        return SysErr(rc);
    }

    size_t nwritten;
    rc = op_->Write(fh->ino, fh->offset, buffer, count, &nwritten);
    if (rc == CURVEFS_ERROR::OK) {
        fh->offset += nwritten;
    }
    return SysErr(rc);
}

int VFS::FSync(int fd) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("fsync (%d): %s", fd, StrErr(rc));
    });

    std::shared_ptr<FileHandler> fh;
    bool yes = handlers_->GetHandler(fd, &fh);
    if (!yes) {
        rc = CURVEFS_ERROR::BAD_FD;
        return SysErr(rc);
    }

    rc = op_->Flush(fh->ino);
    return SysErr(rc);
}

int VFS::Stat(const std::string& path, struct stat* stat) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("stat (%s): %s", path, StrErr(rc));
    });

    Entry entry;
    rc = Lookup(path, true, &entry);
    if (rc != CURVEFS_ERROR::OK) {
        return SysErr(rc);
    }

    // Attr2Stat(&entry.attr, stat);  // FIXME(Wine93):
    return SysErr(rc);
}

int VFS::FStat(int fd, struct stat* stat) {
    CURVEFS_ERROR rc;
    AccessLogGuard log([&](){
        return StrFormat("fstat (%d): %s", fd, StrErr(rc));
    });

    std::shared_ptr<FileHandler> fh;
    bool yes = handlers_->GetHandler(fd, &fh);
    if (!yes) {  // already closed or never opened
        rc = CURVEFS_ERROR::BAD_FD;
        return SysErr(rc);
    }

    AttrOut attrOut;
    rc = op_->GetAttr(fh->ino, &attrOut);
    if (rc == CURVEFS_ERROR::OK) {
        // Attr2Stat(&attrOut.attr, stat);  // FIXME(Wine93):
    }
    return SysErr(rc);
}

CURVEFS_ERROR VFS::Lookup(const std::string& pathname,
                          bool followSymlink,
                          Entry* entry) {
    Ino parent = ROOTINODEID;
    std::vector<std::string> names = filepath::Split(pathname);
    for (const auto& name : names) {  // recursive lookup entry
        bool yes = permission_->Check(parent, name);
        if (!yes) {
            return CURVEFS_ERROR::NO_PERMISSION;
        }

        // lookup
        Ino ino;
        yes = entryCache_->Get(parent, name, &ino);
        if (!yes) {
            EntryOut entryOut;
            auto rc = op_->Lookup(parent, name, &entryOut);
            if (rc != CURVEFS_ERROR::OK) {
                return rc;
            }

            ino = entryOut.attr.inodeid();
            entryCache_->Put(parent, name, ino, entryOut.entryTimeout);
            attrCache_->Put(ino, entryOut.attr, entryOut.attrTimeout);
        }
        entry->ino = ino;

        // getattr
        InodeAttr attr;
        yes = attrCache_->Get(ino, &attr);
        if (!yes) {
            AttrOut attrOut;
            auto rc = op_->GetAttr(ino, &attrOut);
            if (rc != CURVEFS_ERROR::OK) {
                return rc;
            }
            attrCache_->Put(ino, attrOut.attr, attrOut.attrTimeout);
        }
        entry->attr = std::move(attr);

        // parent
        parent = entry->ino;
    }

    if (followSymlink && IsSymlink(entry->attr)) {  // follow symbolic link
        std::string link;
        auto rc = op_->ReadLink(entry->ino, &link);
        if (rc != CURVEFS_ERROR::OK) {
            return rc;
        }
        return Lookup(link, followSymlink, entry);
    }
    return CURVEFS_ERROR::OK;
}

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

