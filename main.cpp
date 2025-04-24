#include "lyf.h"

int
main() {
    lyf::auto_stopwatch sw(lyf::stopwatch::TimeType::ms);

    LOG_INFO("hello {}", "info");
    LOG_WARN("hello {}", "warn");
    LOG_ERROR("hello {}", "error");
    LOG_DEBUG("hello {} in file:{}, line:{}", "debug", __FILE__, __LINE__);

    auto str = lyf::FormatMessage("{} format {}", 1, 2);
    LOG_ONLY(str);

    m_print(str);
    m_debug(str);

    return 0;
}
