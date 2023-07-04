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
 * Created Date: 2023-07-25
 * Author: Jingli Chen (Wine93)
 */

#include "curvefs/src/client/fuse/fuse_ll.h"

namespace curvefs {
namespace client {
namespace fuse {

static const struct fuse_lowlevel_ops curve_fuse_ll_ops = {
    init : fuse_ll_init,
    destroy : fuse_ll_destroy,
    lookup : fuse_ll_lookup,
    forget : 0,
    getattr : fuse_ll_getattr,
    setattr : fuse_ll_setattr,
    readlink : fuse_ll_readlink,
    mknod : fuse_ll_mknod,
    mkdir : fuse_ll_mkdir,
    unlink : fuse_ll_unlink,
    rmdir : fuse_ll_rmdir,
    symlink : fuse_ll_symlink,
    rename : fuse_ll_rename,
    link : fuse_ll_link,
    open : fuse_ll_open,
    read : fuse_ll_read,
    write : fuse_ll_write,
    flush : fuse_ll_flush,
    release : fuse_ll_release,
    fsync : fuse_ll_fsync,
    opendir : FuseOpOpenDir,
    #if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
    readdir : 0,
    #else
    readdir : FuseOpReadDir,
    #endif
    releasedir : fuse_ll_releasedir,
    fsyncdir : 0,
    statfs : fuse_ll_statfs,
    setxattr : FuseOpSetXattr,
    getxattr : FuseOpGetXattr,
    listxattr : FuseOpListXattr,
    removexattr : 0,
    access : 0,
    create : FuseOpCreate,
    getlk : 0,
    setlk : 0,
    bmap : FuseOpBmap,
    #if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
    ioctl : 0,
    poll : 0,
    #endif
    #if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)
    write_buf : 0,
    retrieve_reply : 0,
    forget_multi : 0,
    flock : 0,
    fallocate : 0,
    #endif
    #if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
    readdirplus : FuseOpReadDirPlus,
    #else
    readdirplus : 0,
    #endif
    #if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 4)
    copy_file_range : 0,
    #endif
    #if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 8)
    lseek : 0
    #endif
};

static void passthrough_ll_help(void)
{
	printf(
"    -o writeback           Enable writeback\n"
"    -o no_writeback        Disable write back\n"
"    -o source=/home/dir    Source directory to be mounted\n"
"    -o flock               Enable flock\n"
"    -o no_flock            Disable flock\n"
"    -o xattr               Enable xattr\n"
"    -o no_xattr            Disable xattr\n"
"    -o timeout=1.0         Caching timeout\n"
"    -o timeout=0/1         Timeout is set\n"
"    -o cache=never         Disable cache\n"
"    -o cache=auto          Auto enable cache\n"
"    -o cache=always        Cache always\n");
}


void FuseService::Run() {

}

}  // namespace fuse
}  // namespace client
}  // namespace curvefs
