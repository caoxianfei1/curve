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
 * Created Date: 2023-07-11
 * Author: Jingli Chen (Wine93)
 */

#ifndef CURVEFS_SRC_CLIENT_VFS_CONFIG_H_
#define CURVEFS_SRC_CLIENT_VFS_CONFIG_H_

#include <incbin.h>

#include <map>
#include <string>
#include <memory>
#include <functional>

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX g_
/* Usage: INCBIN(<<LABLE>>, <<FILE>>)
 *
 * Symbols defined by INCBIN
 * ------------------------------------------
 *  const unsigned char        g_client_conf_data[]  // g_<<LABLE>>_data
 *  const unsigned char* const g_client_conf_end;    // g_<<LABEL>>_end
 *  const unsinged int         g_client_conf_size;   // g_<<LABEL>>_size
 */
INCBIN(client_conf, "curvefs/conf/client.conf");

namespace curvefs {
namespace client {
namespace vfs {

class Configure {
 public:
    using IterateHandler = std::function<void(const std::string& key,
                                              const std::string& value)>;
 public:
    Configure() = default;

    static std::shared_ptr<Configure> Default();

    void LoadString(const std::string& in);

    bool Get(const std::string& key, std::string* value);

    void Set(const std::string& key, const std::string& value);

    void Iterate(IterateHandler handler);

 private:
    void LoadLine(const std::string& line);

 private:
    std::map<std::string, std::string> m_;
};

}  // namespace vfs
}  // namespace client
}  // namespace curvefs

#endif  // CURVEFS_SRC_CLIENT_VFS_CONFIG_H_
