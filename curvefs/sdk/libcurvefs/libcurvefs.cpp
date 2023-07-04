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

#include "curvefs/sdk/libcurvefs/libcurvefs.h"

static CurveFSMount* get_instance(uintptr_t instance_ptr) {
    return reinterpret_cast<CurveFSMount*>(instance_ptr);
}

uintptr_t curvefs_create() {
    auto mount = new CurveFSMount();
    return reinterpret_cast<uintptr_t>(mount);
}

void conf_set(uintptr_t instance_ptr, const char* key, const char* value) {
    auto mount = get_instance(instance_ptr);
    return mount->cfg->Set(key, value);
}

int curvefs_mount(uintptr_t instance_ptr,
                  const char* fsname,
                  const char* mountpoint) {
    auto mount = get_instance(instance_ptr);
    return mount->vfs->Mount(fsname, mountpoint, mount->cfg);
}

int curvefs_mkdir(uintptr_t instance_ptr, const char* path, mode_t mode) {
    auto mount = get_instance(instance_ptr);
    return mount->vfs->MkDir(path, mode);
}

int curvefs_rmdir(uintptr_t instance_ptr, const char* path) {
    auto mount = get_instance(instance_ptr);
    return mount->vfs->RmDir(path);
}

