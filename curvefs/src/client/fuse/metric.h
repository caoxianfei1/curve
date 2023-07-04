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

#include <string>

#include "curvefs/src/common/metric_utils.h"

#ifndef CURVEFS_SRC_CLIENT_FUSE_METRIC_H_
#define CURVEFS_SRC_CLIENT_FUSE_METRIC_H_

#define DEFINE_METRICS(seq) END(A seq)
#define BODY(x) LLMetric x = LLMetric(#x);
#define A(x) BODY(x) B
#define B(x) BODY(x) A
#define A_END
#define B_END
#define END(...) END_(__VA_ARGS__)
#define END_(...) __VA_ARGS__##_END


struct FuseLLOpMetric {
    bvar::LatencyRecorder latency;
    bvar::Adder<int64_t> inflightOpNum;
    bvar::Adder<uint64_t> ecount;

    explicit OpMetric(const std::string& name)
        : latency(prefix, name + "_latency"),
          ninflight(prefix, name + "_ninflight"),
          nerror(prefix, name + "_nerror") {}
};

struct OperatorMetric {
    DEFINE_METRICS(
        (lookup) (open) (create) (mknod) (mkdir) (link) (unlink) (rmdir)
        (opendir) (releasedir) (readdir) (rename) (getattr) (setattr)
        (getxattr) (listxattr) (symlink) (readlink) (release) (fsync)
        (flush)(read)(write)
    )

    OperatorMetric GetInstance() {
        static OperatorMetric instance;
        return instance;
    }
};

struct MetricGuard {

    MetricGuard

    InflightGuard iGuard;
    CodeGuard cGuard;
    LatencyUpdater updater;
};

#define MetricGuard(op)

#endif  // CURVEFS_SRC_CLIENT_FUSE_METRIC_H_
