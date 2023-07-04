/*
 *  Copyright (c) 2021 NetEase Inc.
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
 * Project: curve
 * Created Date: Thur May 27 2021
 * Author: xuchaojie
 */

#include <glog/logging.h>

#include <string>
#include <unordered_map>

#include "curvefs/src/client/curve_fuse_op.h"
#include "curvefs/src/client/fuse_common.h"

namespace {
int GetFsInfo(const char* fsName, FsInfo* fsInfo) {
    MdsClientImpl mdsClient;
    MDSBaseClient mdsBase;
    mdsClient.Init(g_fuseClientOption->mdsOpt, &mdsBase);

    std::string fn = (fsName == nullptr) ? "" : fsName;
    FSStatusCode ret = mdsClient.GetFsInfo(fn, fsInfo);
    if (ret != FSStatusCode::OK) {
        if (FSStatusCode::NOT_FOUND == ret) {
            LOG(ERROR) << "The fsName not exist, fsName = " << fsName;
            return -1;
        } else {
            LOG(ERROR) << "GetFsInfo failed, FSStatusCode = " << ret
                       << ", FSStatusCode_Name = " << FSStatusCode_Name(ret)
                       << ", fsName = " << fsName;
            return -1;
        }
    }
    return 0;
}

int InitFuseClient(const struct MountOption *mountOption) {
    g_clientOpMetric = new ClientOpMetric();

    Configuration conf;
    conf.SetConfigPath(mountOption->conf);
    if (!conf.LoadConfig()) {
        LOG(ERROR) << "LoadConfig failed, confPath = " << mountOption->conf;
        return -1;
    }
    if (mountOption->mdsAddr)
        conf.SetStringValue("mdsOpt.rpcRetryOpt.addrs", mountOption->mdsAddr);

    conf.PrintConfig();

    g_fuseClientOption = new FuseClientOption();
    curvefs::client::common::InitFuseClientOption(&conf, g_fuseClientOption);

    std::shared_ptr<FsInfo> fsInfo = std::make_shared<FsInfo>();
    if (GetFsInfo(mountOption->fsName, fsInfo.get()) != 0) {
        return -1;
    }

    std::string fsTypeStr =
        (mountOption->fsType == nullptr) ? "" : mountOption->fsType;
    std::string fsTypeMds;
    if (fsInfo->fstype() == FSType::TYPE_S3) {
       fsTypeMds = "s3";
    } else if (fsInfo->fstype() == FSType::TYPE_VOLUME) {
       fsTypeMds = "volume";
    }

    if (fsTypeMds != fsTypeStr) {
        LOG(ERROR) << "The parameter fstype is inconsistent with mds!";
        return -1;
    } else if (fsTypeStr == "s3") {
        g_ClientInstance = new FuseS3Client();
    } else if (fsTypeStr == "volume") {
        g_ClientInstance = new FuseVolumeClient();
    } else {
        LOG(ERROR) << "unknown fstype! fstype is " << fsTypeStr;
        return -1;
    }

    g_ClientInstance->SetFsInfo(fsInfo);
    CURVEFS_ERROR ret = g_ClientInstance->Init(*g_fuseClientOption);
    if (ret != CURVEFS_ERROR::OK) {
        return -1;
    }
    ret = g_ClientInstance->Run();
    if (ret != CURVEFS_ERROR::OK) {
        return -1;
    }

    ret = g_ClientInstance->SetMountStatus(mountOption);
    if (ret != CURVEFS_ERROR::OK) {
        return -1;
    }

    return 0;
}

void UnInitFuseClient() {
    if (g_ClientInstance) {
        g_ClientInstance->Fini();
        g_ClientInstance->UnInit();
    }
    delete g_ClientInstance;
    delete g_fuseClientOption;
    delete g_clientOpMetric;
}

}  // namespace

void print_option_help(const char* o, const char* msg) {
    printf("    -o %-20s%s\n", o, msg);
}

void extra_options_help() {
    printf("\nExtra options:\n");
    print_option_help("fsname", "[required] name of filesystem to be mounted");
    print_option_help(
        "fstype", "[required] type of filesystem to be mounted (s3/volume)");
    print_option_help("conf", "[required] path of config file");
    printf("    --mdsAddr              mdsAddr of curvefs cluster\n");
}

std::string match_any_pattern(
    const std::unordered_map<std::string, char**>& patterns, const char* src) {
    size_t src_len = strlen(src);
    for (const auto& pair : patterns) {
        const auto& pattern = pair.first;
        if (pattern.length() < src_len &&
            strncmp(pattern.c_str(), src, pattern.length()) == 0) {
            return pattern;
        }
    }
    return {};
}

void parse_option(int argc, char** argv, int* parsed_argc_p, char** parsed_argv,
                  struct MountOption* opts) {
    // add support for parsing option value with comma(,)
    std::unordered_map<std::string, char**> patterns = {
        {"--mdsaddr=", &opts->mdsAddr}};
    for (int i = 0, j = 0; j < argc; j++) {
        std::string p = match_any_pattern(patterns, argv[j]);
        int p_len = p.length();
        int src_len = strlen(argv[j]);
        if (p_len) {
            if (*patterns[p]) {
                free(*patterns[p]);
            }
            *patterns[p] = reinterpret_cast<char*>(
                malloc(sizeof(char) * (src_len - p_len + 1)));
            memcpy(*patterns[p], argv[j] + p_len, src_len - p_len);
            (*patterns[p])[src_len - p_len] = '\0';
            *parsed_argc_p = *parsed_argc_p - 1;
        } else {
            parsed_argv[i] =
                reinterpret_cast<char*>(malloc(sizeof(char) * (src_len + 1)));
            memcpy(parsed_argv[i], argv[j], src_len);
            parsed_argv[i][src_len] = '\0';
            i++;
        }
    }
}

void free_parsed_argv(char** parsed_argv, int alloc_size) {
    for (int i = 0; i < alloc_size; i++) {
        free(parsed_argv[i]);
    }
    free(parsed_argv);
}

int main(int argc, char *argv[]) {
    struct MountOption mOpts = {0};
    int parsed_argc = argc;
    char** parsed_argv = reinterpret_cast<char**>(malloc(sizeof(char*) * argc));
    parse_option(argc, argv, &parsed_argc, parsed_argv, &mOpts);

    struct fuse_args args = FUSE_ARGS_INIT(parsed_argc, parsed_argv);
    struct fuse_session *se;
    struct fuse_cmdline_opts opts;
    struct fuse_loop_config config;
    int ret = -1;

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        printf(
            "usage: %s -o conf=/etc/curvefs/client.conf -o fsname=testfs \\\n"
            "       -o fstype=s3 [--mdsaddr=1.1.1.1:1234,2.2.2.2:1234] \\\n"
            "       [OPTIONS] <mountpoint>\n",
            argv[0]);
        printf("Fuse Options:\n");
        fuse_cmdline_help();
        fuse_lowlevel_help();
        extra_options_help();
        ret = 0;
        goto err_out1;
    } else if (opts.show_version) {
        printf("FUSE library version %s\n", fuse_pkgversion());
        fuse_lowlevel_version();
        ret = 0;
        goto err_out1;
    }

    if (opts.mountpoint == NULL) {
        printf("required option is missing: mountpoint\n");
        ret = 1;
        goto err_out1;
    }

    if (fuse_opt_parse(&args, &mOpts, mount_opts, NULL)== -1)
        return 1;

    mOpts.mountPoint = opts.mountpoint;

    if (mOpts.conf == NULL || mOpts.fsName == NULL || mOpts.fsType == NULL) {
        printf(
            "one of required options is missing. conf, fsname, fstype are "
            "required.\n");
        ret = 1;
        goto err_out1;
    }

    printf("Begin to mount fs %s to %s\n", mOpts.fsName, mOpts.mountPoint);

    se = fuse_session_new(&args, &curve_ll_oper,
                  sizeof(curve_ll_oper), &mOpts);
    if (se == NULL)
        goto err_out1;

    if (fuse_set_signal_handlers(se) != 0)
        goto err_out2;

    if (fuse_session_mount(se, opts.mountpoint) != 0)
        goto err_out3;

    fuse_daemonize(opts.foreground);

    if (InitLog(mOpts.conf, argv[0]) < 0) {
        printf("Init log failed, confpath = %s\n", mOpts.conf);
    }

    ret = InitFuseClient(&mOpts);
    if (ret < 0) {
        LOG(ERROR) << "init fuse client fail, conf = " << mOpts.conf;
        goto err_out4;
    }

    LOG(INFO) << "fuse start loop, singlethread = " << opts.singlethread
              << ", max_idle_threads = " << opts.max_idle_threads;

    /* Block until ctrl+c or fusermount -u */
    if (opts.singlethread) {
        ret = fuse_session_loop(se);
    } else {
        config.clone_fd = opts.clone_fd;
        config.max_idle_threads = opts.max_idle_threads;
        ret = fuse_session_loop_mt(se, &config);
    }

err_out4:
    fuse_session_unmount(se);
err_out3:
    fuse_remove_signal_handlers(se);
err_out2:
    fuse_session_destroy(se);
err_out1:
    UnInitFuseClient();
    free(opts.mountpoint);
    free_parsed_argv(parsed_argv, argc);
    fuse_opt_free_args(&args);

    return ret ? 1 : 0;
}
