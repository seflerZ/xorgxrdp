/*
Copyright 2013-2017 Jay Sorg

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this should be before all X11 .h files */
#include <xorg-server.h>
#include <xorgVersion.h>

/* all driver need this */
#include <xf86.h>
#include <xf86_OSproc.h>

#include "rdp.h"
#include "rdpDraw.h"
#include "rdpInput.h"
#include "rdpMisc.h"

#define LOG_LEVEL 1
#define LLOGLN(_level, _args) \
    do { if (_level < LOG_LEVEL) { ErrorF _args ; ErrorF("\n"); } } while (0)

#define MAX_INPUT_PROC 4

struct input_proc_list
{
    int type;
    rdpInputEventProcPtr proc;
};

static struct input_proc_list g_input_proc[MAX_INPUT_PROC];

/******************************************************************************/
int
rdpRegisterInputCallback(int type, rdpInputEventProcPtr proc)
{
    LLOGLN(0, ("rdpRegisterInputCallback: type %d proc %p", type, proc));
    if (type == 0)
    {
        g_input_proc[0].proc = proc;
    }
    else if (type == 1)
    {
        g_input_proc[1].proc = proc;
    }
    else
    {
        return 1;
    }
    return 0;
}

/******************************************************************************/
int
rdpUnregisterInputCallback(rdpInputEventProcPtr proc)
{
    int index;

    LLOGLN(0, ("rdpUnregisterInputCallback: proc %p", proc));
    for (index = 0; index < MAX_INPUT_PROC; index++)
    {
        if (g_input_proc[index].proc == proc)
        {
            g_input_proc[index].proc = 0;
            return 0;
        }
    }
    return 1;
}

/******************************************************************************/
int
rdpInputKeyboardEvent(rdpPtr dev, int msg,
                      long param1, long param2,
                      long param3, long param4)
{
    dev->last_event_time_ms = GetTimeInMillis();

    if (g_input_proc[0].proc != 0)
    {
        return g_input_proc[0].proc(dev, msg, param1, param2, param3, param4);
    }
    return 0;
}

/******************************************************************************/
int
rdpInputMouseEvent(rdpPtr dev, int msg,
                   long param1, long param2,
                   long param3, long param4)
{
    dev->last_event_time_ms = GetTimeInMillis();
    if (msg == 108 || msg == 110 || msg == 107 || msg == 109) {
        if (dev->last_event_time_ms - dev->last_wheel_time_ms < 50)
        {
            return 0;
        }
    }

    if (msg == 107 || msg == 109) {
        dev->last_wheel_time_ms = dev->last_event_time_ms;
    }

    if (g_input_proc[1].proc != 0)
    {
        return g_input_proc[1].proc(dev, msg, param1, param2, param3, param4);
    }
    return 0;
}

/******************************************************************************/
/* called when module loads */
int
rdpInputInit(void)
{
    g_memset(g_input_proc, 0, sizeof(g_input_proc));
    return 0;
}
