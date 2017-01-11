#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>
#include <Carbon/Carbon.h>

#define internal static
extern bool CGSIsSecureEventInputSet(void);
#define IsSecureKeyboardEntryEnabled CGSIsSecureEventInputSet

internal CFMachPortRef CevEventTap;
internal const char *CevVersion = "0.0.1";

internal inline void
Error(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    vfprintf(stderr, Format, Args);
    va_end(Args);

    exit(EXIT_FAILURE);
}

#define osx_event_mask_ctrl 0x00040000
internal void
CheckForExitSignal(CGEventFlags Flags, CGKeyCode Key)
{
    if(Flags & osx_event_mask_ctrl)
    {
        if(Key == kVK_ANSI_C)
        {
            exit(0);
        }
    }
}

internal inline void
PrintKeyEvent(CGEventFlags Flags, CGKeyCode Key)
{
    printf("keycode: 0x%.2X\n\n", Key);
}

internal CGEventRef
KeyCallback(CGEventTapProxy Proxy, CGEventType Type, CGEventRef Event, void *Context)
{
    switch(Type)
    {
        case kCGEventTapDisabledByTimeout:
        case kCGEventTapDisabledByUserInput:
        {
            printf("cev: restarting event-tap\n");
            CGEventTapEnable(CevEventTap, true);
        } break;
        case kCGEventKeyDown:
        {
            CGEventFlags Flags = CGEventGetFlags(Event);
            CGKeyCode Key = CGEventGetIntegerValueField(Event, kCGKeyboardEventKeycode);
            CheckForExitSignal(Flags, Key);
            PrintKeyEvent(Flags, Key);
            return NULL;
        } break;
        case kCGEventFlagsChanged:
        {
            CGEventFlags Flags = CGEventGetFlags(Event);
            CGKeyCode Key = CGEventGetIntegerValueField(Event, kCGKeyboardEventKeycode);
            PrintKeyEvent(Flags, Key);
        } break;
    }

    return Event;
}

internal inline void
ConfigureRunLoop(void)
{
    CGEventMask CevEventMask = (1 << kCGEventKeyDown) |
                               (1 << kCGEventFlagsChanged);
    CevEventTap = CGEventTapCreate(kCGSessionEventTap,
                                   kCGHeadInsertEventTap,
                                   kCGEventTapOptionDefault,
                                   CevEventMask,
                                   KeyCallback,
                                   NULL);

    if(!CevEventTap || !CGEventTapIsEnabled(CevEventTap))
        Error("cev: could not create event-tap, try running as root!\n");

    CFRunLoopAddSource(CFRunLoopGetMain(),
                       CFMachPortCreateRunLoopSource(kCFAllocatorDefault, CevEventTap, 0),
                       kCFRunLoopCommonModes);
}

internal inline bool
ParseArguments(int Count, char **Args)
{
    int Option;
    const char *Short = "v";
    struct option Long[] =
    {
        { "version", no_argument, NULL, 'v' },
        { NULL, 0, NULL, 0 }
    };

    while((Option = getopt_long(Count, Args, Short, Long, NULL)) != -1)
    {
        switch(Option)
        {
            case 'v':
            {
                printf("cev version %s\n", CevVersion);
                return true;
            } break;
        }
    }

    return false;
}

internal inline bool
CheckPrivileges(void)
{
    bool Result = false;
    const void *Keys[] = { kAXTrustedCheckOptionPrompt };
    const void *Values[] = { kCFBooleanFalse };

    CFDictionaryRef Options;
    Options = CFDictionaryCreate(kCFAllocatorDefault,
                                 Keys, Values, sizeof(Keys) / sizeof(*Keys),
                                 &kCFCopyStringDictionaryKeyCallBacks,
                                 &kCFTypeDictionaryValueCallBacks);

    Result = AXIsProcessTrustedWithOptions(Options);
    CFRelease(Options);

    return Result;
}

int main(int Count, char **Args)
{
    if(ParseArguments(Count, Args))
        return EXIT_SUCCESS;

    if(IsSecureKeyboardEntryEnabled())
        Error("cev: secure keyboard entry is enabled! Terminating.\n");

    if(getuid() != 0 && !CheckPrivileges())
        Error("cev: must be run with accessibility access, or as root.\n");

    ConfigureRunLoop();
    CFRunLoopRun();

    return EXIT_SUCCESS;
}
