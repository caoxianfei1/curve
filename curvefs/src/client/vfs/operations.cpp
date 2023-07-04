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

#include "curvefs/src/client/vfs/meta.h"
#include "curvefs/src/client/vfs/operations.h"

namespace curvefs {
namespace client {
namespace vfs {


OperationsImpl::OperationsImpl(std::shared_ptr<FuseClient> client)
    : client_(client),
      fs_(client->GetFileSystem()) {}

fuse_req_t OperationsImpl::DummyRequest() {
    return fuse_req_t();
}

fuse_file_info OperationsImpl::DummyFileInfo() {
    return fuse_file_info();
}

CURVEFS_ERROR OperationsImpl::Lookup(Ino parent,
                                     const std::string& name,
                                     EntryOut* entryOut) {
    auto req = DummyRequest();
    auto rc = client_->FuseOpLookup(req, parent, name.c_str(), entryOut);
    if (rc == CURVEFS_ERROR::OK) {
        fs_->SetEntryTimeout(entryOut);
    }
    return rc;
}

CURVEFS_ERROR OperationsImpl::GetAttr(Ino ino, AttrOut* attrOut) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    auto rc = client_->FuseOpGetAttr(req, ino, &fi, attrOut);
    if (rc == CURVEFS_ERROR::OK) {
        fs_->SetAttrTimeout(attrOut);
    }
    return rc;
}

CURVEFS_ERROR OperationsImpl::MkDir(Ino parent,
                                    const std::string& name,
                                    uint32_t mode) {
    EntryOut entryOut;
    auto req = DummyRequest();
    return client_->FuseOpMkDir(req, parent, name.c_str(), mode, &entryOut);
}

CURVEFS_ERROR OperationsImpl::OpenDir(Ino ino, uint64_t* fh) {
    auto fi = DummyFileInfo();
    CURVEFS_ERROR rc = fs_->OpenDir(ino, &fi);
    if (rc == CURVEFS_ERROR::OK) {
        *fh = fi.fh;
    }
    return rc;
}

CURVEFS_ERROR OperationsImpl::ReadDir(Ino ino,
                                      uint64_t fh,
                                      std::shared_ptr<DirEntryList>* entries) {
    // DON'T SHOCK WHY WE READDIR BY FILESYSTEM INSTED OF CLIENT :)
    auto fi = DummyFileInfo();
    fi.fh = fh;
    return fs_->ReadDir(ino, &fi, entries);
}

CURVEFS_ERROR OperationsImpl::CloseDir(Ino ino) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    return client_->FuseOpRelease(req, ino, &fi);
}

CURVEFS_ERROR OperationsImpl::RmDir(Ino parent, const std::string& name) {
    auto req = DummyRequest();
    return client_->FuseOpRmDir(req, parent, name.c_str());
}

CURVEFS_ERROR OperationsImpl::Create(Ino parent,
                                     const std::string& name,
                                     uint32_t mode) {
    auto req = DummyRequest();
    EntryOut entryOut;
    auto rc = client_->FuseOpMkNod(req, parent, name.c_str(), mode, 0,
                                   &entryOut);
    return rc;
}

CURVEFS_ERROR OperationsImpl::Open(Ino ino, int flags) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    FileOut fileOut;
    fi.flags = flags;
    return client_->FuseOpOpen(req, ino, &fi, &fileOut);
}

CURVEFS_ERROR OperationsImpl::Write(Ino ino,
                                    off_t offset,
                                    char* buffer,
                                    size_t size,
                                    size_t* nwritten) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    FileOut fileOut;
    auto rc = client_->FuseOpWrite(req, ino, buffer, size,
                                  offset, &fi, &fileOut);
    if (rc == CURVEFS_ERROR::OK) {
        *nwritten = fileOut.nwritten;
    } else {
        *nwritten = 0;
    }
    return rc;
}

CURVEFS_ERROR OperationsImpl::Read(Ino ino,
                                   off_t offset,
                                   char* buffer,
                                   size_t size,
                                   size_t* nread) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    return client_->FuseOpRead(req, ino, size, offset, &fi, buffer, nread);
}

CURVEFS_ERROR OperationsImpl::Unlink(Ino parent, const std::string& name) {
    auto req = DummyRequest();
    return client_->FuseOpUnlink(req, parent, name.c_str());
}

CURVEFS_ERROR OperationsImpl::Flush(Ino ino) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    return client_->FuseOpFlush(req, ino, &fi);
}

CURVEFS_ERROR OperationsImpl::Close(Ino ino) {
    auto req = DummyRequest();
    auto fi = DummyFileInfo();
    return client_->FuseOpRelease(req, ino, &fi);
}

}  // namespace vfs
}  // namespace client
}  // namespace curvefs
