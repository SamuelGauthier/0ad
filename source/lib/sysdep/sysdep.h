/**
 * =========================================================================
 * File        : sysdep.h
 * Project     : 0 A.D.
 * Description : various system-specific function implementations
 * =========================================================================
 */

// license: GPL; see lib/license.txt

#ifndef INCLUDED_SYSDEP
#define INCLUDED_SYSDEP

#include "lib/debug.h"	// ErrorReaction

#include <cstdarg>	// needed for sys_vsnprintf


//
// output
//

/**
 * display a message.
 *
 * @param caption title message
 * @param msg message contents
 *
 * implemented as a MessageBox on Win32 and printf on Unix.
 * called from debug_display_msgw.
 **/
extern void sys_display_msg(const wchar_t* caption, const wchar_t* msg);

/**
 * show the error dialog.
 *
 * @param text to display (practically unlimited length)
 * @param flags: see DebugDisplayErrorFlags.
 * @return ErrorReaction (except ER_EXIT, which is acted on immediately)
 *
 * called from debug_display_error unless overridden by means of
 * ah_display_error.
 **/
extern ErrorReaction sys_display_error(const wchar_t* text, size_t flags);


//
// misc
//

/**
 * sys_vsnprintf: doesn't quite follow the standard for vsnprintf, but works
 * better across compilers:
 * - handles positional parameters and %lld
 * - always null-terminates the buffer
 * - returns -1 on overflow (if the output string (including null) does not fit in the buffer)
 **/
extern int sys_vsnprintf(char* buffer, size_t count, const char* format, va_list argptr);

/**
 * describe the current OS error state.
 * 
 * @param err: if not 0, use that as the error code to translate; otherwise,
 * uses GetLastError or similar.
 * @param buf output buffer
 * @param max_chars
 *
 * rationale: it is expected to be rare that OS return/error codes are
 * actually seen by user code, but we leave the possibility open.
 **/
extern LibError sys_error_description_r(int err, char* buf, size_t max_chars);

/**
 * determine filename of the module to whom an address belongs.
 *
 * @param path receives full path to module or L"" on error.
 * @param max_chars
 *
 * note: this is useful for handling exceptions in other modules.
 **/
void sys_get_module_filename(void* addr, wchar_t* path, size_t max_chars);

/**
 * get path to the current executable.
 *
 * @param n_path receives the full native path.
 * @param max_chars
 *
 * this is useful for determining installation directory, e.g. for VFS.
 **/
extern LibError sys_get_executable_name(char* n_path, size_t max_chars);

/**
 * have the user choose a directory via OS dialog.
 *
 * @param n_path receives the full native path.
 * @param max_chars must be at least PATH_MAX due to a Win32 limitation.
 **/
extern LibError sys_pick_directory(char* n_path, size_t max_chars);

/**
 * return the largest sector size [bytes] of any storage medium
 * (HD, optical, etc.) in the system.
 * 
 * this may be a bit slow to determine (iterates over all drives),
 * but caches the result so subsequent calls are free.
 * (caveat: device changes won't be noticed during this program run)
 * 
 * sector size is relevant because Windows aio requires all IO
 * buffers, offsets and lengths to be a multiple of it. this requirement
 * is also carried over into the vfs / file.cpp interfaces for efficiency
 * (avoids the need for copying to/from align buffers).
 * 
 * waio uses the sector size to (in some cases) align IOs if
 * they aren't already, but it's also needed by user code when
 * aligning their buffers to meet the requirements.
 * 
 * the largest size is used so that we can read from any drive. while this
 * is a bit wasteful (more padding) and requires iterating over all drives,
 * it is the only safe way: this may be called before we know which
 * drives will be needed, and hardlinks may confuse things.
 **/
extern size_t sys_max_sector_size();

/**
 * directory separation character
 **/
#if OS_WIN
# define SYS_DIR_SEP '\\'
#else
# define SYS_DIR_SEP '/'
#endif

#endif	// #ifndef INCLUDED_SYSDEP
