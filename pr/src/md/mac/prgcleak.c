/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Netscape security libraries.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */


/* Contributor(s):
 *   Patrick Beard <beard@netscape.com>
 */

/*
 * prgcleak.c
 */

#ifdef GC_LEAK_DETECTOR

/* for FILE */
#include <stdio.h>

/* NSPR stuff */
#include "generic_threads.h"
#include "primpl.h"

extern FILE *GC_stdout, *GC_stderr;

extern void GC_gcollect(void);
extern void GC_clear_roots(void);

static PRStatus PR_CALLBACK scanner(PRThread* t, void** baseAddr,
                                    PRUword count, void* closure)
{
    char* begin = (char*)baseAddr;
    char* end = (char*)(baseAddr + count);
    GC_mark_range_proc marker = (GC_mark_range_proc) closure;
    marker(begin, end);
    return PR_SUCCESS;
}

static void mark_all_stacks(GC_mark_range_proc marker)
{
    PR_ScanStackPointers(&scanner, (void *)marker);
}

static void locker(void* mutex)
{
    if (_PR_MD_CURRENT_CPU())
        PR_Lock(mutex);
}

static void unlocker(void* mutex)
{
    if (_PR_MD_CURRENT_CPU())
        PR_Unlock(mutex);
}

static void stopper(void* unused)
{
    if (_PR_MD_CURRENT_CPU())
        PR_SuspendAll();
}

static void starter(void* unused)
{
    if (_PR_MD_CURRENT_CPU())
        PR_ResumeAll();
}

void _PR_InitGarbageCollector()
{
    void* mutex;

    /* redirect GC's stderr to catch startup leaks. */
    GC_stderr = fopen("StartupLeaks", "w");

    mutex = PR_NewLock();
    PR_ASSERT(mutex != NULL);

    GC_generic_init_threads(&mark_all_stacks, mutex, 
            &locker, &unlocker,
            &stopper, &starter);
}

void _PR_ShutdownGarbageCollector()
{
    /* do anything you need to shut down the collector. */
}

#endif /* GC_LEAK_DETECTOR */
