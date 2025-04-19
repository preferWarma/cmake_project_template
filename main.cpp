#include "lyf_all.h"

int
main() {
    lyf::auto_stopwatch sw(lyf::stopwatch::TimeType::ms);

    LOG_INFO("hello {}", "info");
    LOG_WARN("hello {}", "warn");
    LOG_ERROR("hello {}", "error");
    LOG_DEBUG("hello {}", "debug");

    auto str = lyf::FormatMessage("{} format {}", 1, 2);
    LOG_ONLY(str);

    return 0;
}
