#ifndef _TRACE_H_
#define _TRACE_H_

// Include WPP headers
#include <WppRecorder.h>

// Define the control GUID and trace flags
#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        myDriverTraceGuid, (84bdb2e9, 829e, 41b3, b891, 02f454bc2bd7), \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                   \
    )

// Define the macros needed for logging
#define WPP_FLAG_LEVEL_LOGGER(Flags, Level)    WPP_LEVEL_LOGGER(Flags)
#define WPP_FLAG_LEVEL_ENABLED(Flags, Level)   (WPP_LEVEL_ENABLED(Flags) && WPP_CONTROL(WPP_BIT_ ## Flags).Level >= Level)

// Configuration for TraceEvents function
// This block is scanned by the WPP preprocessor
// begin_wpp config
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp

#endif // _TRACE_H_
