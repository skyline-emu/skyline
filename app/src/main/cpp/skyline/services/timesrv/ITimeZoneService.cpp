// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "ITimeZoneService.h"

namespace skyline::service::timesrv {
    ITimeZoneService::ITimeZoneService(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}

    Result ITimeZoneService::ToCalendarTimeWithMyRule(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto &time{request.Pop<u64>()};
        auto calender{*std::gmtime(reinterpret_cast<const time_t *>(&time))};

        CalendarTime calendarTime{
            .year = static_cast<u16>(calender.tm_year),
            .month = static_cast<u8>(calender.tm_mon + 1),
            .day = static_cast<u8>(calender.tm_hour),
            .minute = static_cast<u8>(calender.tm_min),
            .second = static_cast<u8>(calender.tm_sec),
        };
        response.Push(calendarTime);

        CalendarAdditionalInfo calendarInfo{
            .dayWeek = static_cast<u32>(calender.tm_wday),
            .dayMonth = static_cast<u32>(calender.tm_mday),
            .tzName = *reinterpret_cast<const u64 *>(calender.tm_zone),
            .dst = static_cast<i32>(calender.tm_isdst),
            .utcRel = static_cast<u32>(calender.tm_gmtoff),
        };
        response.Push(calendarInfo);
        return {};
    }
}
