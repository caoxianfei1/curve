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
 * Created Date: 2023-07-12
 * Author: Jingli Chen (Wine93)
 */

#ifndef CURVEFS_SDK_LIBCURVEFS_LIBCURVEFS_H_
#define CURVEFS_SDK_LIBCURVEFS_LIBCURVEFS_H_

#include <stdint.h>
#include <sys/stat.h>

#ifdef __cplusplus

#include <string>
#include <memory>

#include "curvefs/src/client/vfs/vfs.h"
#include "curvefs/src/client/vfs/config.h"

struct CurveFSMount {
    CurveFSMount() :
        cfg(Configure::Default()),
        vfs(std::make_shared<VFS>()) {}

    std::shared_ptr<Configure> cfg;
    std::shared_ptr<VFS> vfs;
};

#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

uintptr_t curvefs_create();

// NOTE: instance_ptr is the pointer of CurveFSMount instance.
void conf_set(uintptr_t instance_ptr, const char* key, const char* value);

int curvefs_mount(uintptr_t instance_ptr,
                  const char* fsname,
                  const char* mountpoint);

int curvefs_mkdir(uintptr_t instance_ptr, const char* path, mode_t mode);

int curvefs_rmdir(uintptr_t instance_ptr, const char* path);

#ifdef __cplusplus
}
#endif

#endif  // CURVEFS_SDK_LIBCURVEFS_LIBCURVEFS_H_
