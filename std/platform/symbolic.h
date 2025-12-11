#ifndef SYMBOLIC_H
#define SYMBOLIC_H

#include "core/utils.h"
#include "platform.h"

enum Axis {
        Axis_X,
        Axis_Y,
        Axis_Z,
        Axis_W,
};

enum Side {
        Side_Min,
        Side_Max,
};

typedef enum OperatingSystem {
        OperatingSystem_Windows,
        OperatingSystem_MacOS,
        OperatingSystem_Linux,
        OperatingSystem_FreeBSD,
        OperatingSystem_NetBSD,
        OperatingSystem_OpenBSD,
        OperatingSystem_Illumos,
        OperatingSystem_Unix,
        OperatingSystem_Unknown,
        OperatingSystem_Count,
} OperatingSystem;

typedef enum Architecture {
        Architecture_X86_64,
        Architecture_X86_32,
        Architecture_ARM64,
        Architecture_ARM32,
        Architecture_Unknown,
        Architecture_Count,
} Architecture;

typedef enum Month {
        Month_January,
        Month_February,
        Month_March,
        Month_April,
        Month_May,
        Month_June,
        Month_July,
        Month_August,
        Month_September,
        Month_October,
        Month_November,
        Month_December,
} Month;

typedef enum DayOfWeek {
        DayOfWeek_Monday,
        DayOfWeek_Tuesday,
        DayOfWeek_Wednesday,
        DayOfWeek_Thursday,
        DayOfWeek_Friday,
        DayOfWeek_Saturday,
        DayOfWeek_Sunday,
} DayOfWeek;

#define MonthsInYear 12
#define DaysInWeek   7

static const char* OperatingSystemName[] = {
    [OperatingSystem_Windows] = "Windows",
    [OperatingSystem_MacOS]   = "MacOS",
    [OperatingSystem_Linux]   = "Linux",
    [OperatingSystem_FreeBSD] = "FreeBSD",
    [OperatingSystem_NetBSD]  = "NetBSD",
    [OperatingSystem_OpenBSD] = "OpenBSD",
    [OperatingSystem_Illumos] = "Illumos",
    [OperatingSystem_Unix]    = "Unix",
    [OperatingSystem_Unknown] = "Unknown",
};

static const char* ArchitectureName[] = {
    [Architecture_X86_64]  = "X86_64",
    [Architecture_X86_32]  = "X86_32",
    [Architecture_ARM64]   = "ARM64",
    [Architecture_ARM32]   = "ARM32",
    [Architecture_Unknown] = "Unknown",
};

static const char* MonthShort[] = {
    [Month_January]   = "Jan",
    [Month_February]  = "Feb",
    [Month_March]     = "Mar",
    [Month_April]     = "Apr",
    [Month_May]       = "May",
    [Month_June]      = "Jun",
    [Month_July]      = "Jul",
    [Month_August]    = "Aug",
    [Month_September] = "Sep",
    [Month_October]   = "Oct",
    [Month_November]  = "Nov",
    [Month_December]  = "Dec",
};

static const char* MonthLong[] = {
    [Month_January]   = "January",
    [Month_February]  = "February",
    [Month_March]     = "March",
    [Month_April]     = "April",
    [Month_May]       = "May",
    [Month_June]      = "June",
    [Month_July]      = "July",
    [Month_August]    = "August",
    [Month_September] = "September",
    [Month_October]   = "October",
    [Month_November]  = "November",
    [Month_December]  = "December",
};

static const char* DayOfWeekShort[] = {
    [DayOfWeek_Monday]    = "Mon",
    [DayOfWeek_Tuesday]   = "Tue",
    [DayOfWeek_Wednesday] = "Wed",
    [DayOfWeek_Thursday]  = "Thu",
    [DayOfWeek_Friday]    = "Fri",
    [DayOfWeek_Saturday]  = "Sat",
    [DayOfWeek_Sunday]    = "Sun",
};

static const char* DayOfWeekLong[] = {
    [DayOfWeek_Monday]    = "Monday",
    [DayOfWeek_Tuesday]   = "Tuesday",
    [DayOfWeek_Wednesday] = "Wednesday",
    [DayOfWeek_Thursday]  = "Thursday",
    [DayOfWeek_Friday]    = "Friday",
    [DayOfWeek_Saturday]  = "Saturday",
    [DayOfWeek_Sunday]    = "Sunday",
};

static inline const char* MonthToStringShort(Month month) {
        return (month >= 0 && month < MonthsInYear) ? MonthShort[month]
                                                    : "Unknown";
}

static inline const char* MonthToStringLong(Month month) {
        return (month >= 0 && month < MonthsInYear) ? MonthLong[month]
                                                    : "Unknown";
}

static inline const char* DayOfWeekToStringShort(DayOfWeek day) {
        return (day >= 0 && day < DaysInWeek) ? DayOfWeekShort[day] : "Unknown";
}

static inline const char* DayOfWeekToStringLong(DayOfWeek day) {
        return (day >= 0 && day < DaysInWeek) ? DayOfWeekLong[day] : "Unknown";
}

static inline const char* OperatingSystemToString(OperatingSystem osv) {
        return (osv >= 0 && osv < OperatingSystem_Count)
                   ? OperatingSystemName[osv]
                   : "Unknown";
}

static inline const char* ArchitectureToString(Architecture arch) {
        return (arch >= 0 && arch < Architecture_Count) ? ArchitectureName[arch]
                                                        : "Unknown";
}

static inline OperatingSystem OperatingSystemFromContext(void) {
        OperatingSystem result = OperatingSystem_Unknown;

#if defined BASE_OS_WINDOWS
        result = OperatingSystem_Windows;
#elif defined BASE_OS_MACOS
        result = OperatingSystem_MacOS;
#elif defined BASE_OS_LINUX
        result = OperatingSystem_Linux;
#elif defined BASE_OS_FREEBSD
        result = OperatingSystem_FreeBSD;
#elif defined BASE_OS_NETBSD
        result = OperatingSystem_NetBSD;
#elif defined BASE_OS_OPENBSD
        result = OperatingSystem_OpenBSD;
#elif defined BASE_OS_ILLUMOS
        result = OperatingSystem_Illumos;
#elif defined BASE_OS_UNIX
        result = OperatingSystem_Unix;
#endif

        return result;
}

static inline Architecture ArchitectureFromContext(void) {
        Architecture result = Architecture_Unknown;
#if defined BASE_ARCH_X86_64
        result = Architecture_X86_64;
#elif defined BASE_ARCH_X86_32
        result = Architecture_X86_32;
#elif defined BASE_ARCH_ARM64
        result = Architecture_ARM64;
#elif defined BASE_ARCH_ARM32
        result = Architecture_ARM32;
#endif
        return result;
}

#endif
