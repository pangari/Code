#pragma once

#include <sys/time.h>
#include <stdint.h>

#include <string>

class TimeNow final
{
    uint64_t usec_;

public:
    TimeNow()
    {
        timeval tv;
        gettimeofday(&tv, nullptr);
        usec_ = tv.tv_sec * 1000000L + tv.tv_usec;
    }

    operator uint64_t() const { return usec_; }
    //operator std::string() const { return std::to_string(usec_); }
    bool operator==(const TimeNow& right) const { return usec_ == right.usec_; }
    bool operator<(const TimeNow& right) const { return usec_ < right.usec_; }
    bool operator<=(const TimeNow& right) const { return usec_ <= right.usec_; }
    bool operator>(const TimeNow& right) const { return usec_ > right.usec_; }
    bool operator>=(const TimeNow& right) const { return usec_ >= right.usec_; }
};
