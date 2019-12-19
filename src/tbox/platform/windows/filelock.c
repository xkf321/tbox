/*!The Treasure Box Library
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Copyright (C) 2009-2020, TBOOX Open Source Group.
 *
 * @author      ruki
 * @file        filelock.c
 * @ingroup     platform
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../filelock.h"
#include "../../libc/libc.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

// the file lock type
typedef struct __tb_filelock_t
{
    // the file reference
    tb_file_ref_t   file;

    // is owner?
    tb_bool_t       owner;

}tb_filelock_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static tb_filelock_ref_t tb_filelock_init_impl(tb_file_ref_t file, tb_bool_t owner)
{
    // check
    tb_assert_and_check_return_val(file, tb_null);

    tb_filelock_t* lock = tb_null;
    do
    {
        // init lock
        lock = tb_malloc0_type(tb_filelock_t);
        tb_assert_and_check_break(lock);

        lock->file  = file;
        lock->owner = owner;

    } while (0);
    return (tb_filelock_ref_t)lock;
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_filelock_ref_t tb_filelock_init(tb_file_ref_t file)
{
    return tb_filelock_init_impl(file, tb_false);
}
tb_filelock_ref_t tb_filelock_init_from_path(tb_char_t const* path, tb_size_t mode)
{
    tb_assert_and_check_return_val(path, tb_null);
    return tb_filelock_init_impl(tb_file_init(path, mode), tb_true);
}
tb_void_t tb_filelock_exit(tb_filelock_ref_t self)
{
    // check
    tb_filelock_t* lock = (tb_filelock_t*)self;
    tb_assert_and_check_return(lock);

    // exit file
    if (lock->file && lock->owner) tb_file_exit(lock->file);
    lock->file = tb_null;

    // exit lock
    tb_free(lock);
}
tb_bool_t tb_filelock_enter(tb_filelock_ref_t self, tb_size_t mode)
{
    // check
    tb_filelock_t* lock = (tb_filelock_t*)self;
    tb_assert_and_check_return_val(lock && lock->file, tb_false);

    // init overlap
    OVERLAPPED overlap; 
    tb_memset(&overlap, 0, sizeof(overlap));

    // lock it
    DWORD flags = (mode == TB_FILELOCK_MODE_EX)? LOCKFILE_EXCLUSIVE_LOCK : 0;
    return (tb_bool_t)LockFileEx((HANDLE)lock->file, flags, 0, 1, 0, &overlap);
}
tb_bool_t tb_filelock_enter_try(tb_filelock_ref_t self, tb_size_t mode)
{
    // check
    tb_filelock_t* lock = (tb_filelock_t*)self;
    tb_assert_and_check_return_val(lock && lock->file, tb_false);

    // init overlap
    OVERLAPPED overlap; 
    tb_memset(&overlap, 0, sizeof(overlap));

    // try to lock it
    DWORD flags = (mode == TB_FILELOCK_MODE_EX)? LOCKFILE_EXCLUSIVE_LOCK : 0;
    return (tb_bool_t)LockFileEx((HANDLE)lock->file, flags | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &overlap);
}
tb_bool_t tb_filelock_leave(tb_filelock_ref_t self)
{
    // check
    tb_filelock_t* lock = (tb_filelock_t*)self;
    tb_assert_and_check_return_val(lock && lock->file, tb_false);

    // unlock
    return (tb_bool_t)UnlockFile((HANDLE)lock->file, 0, 0, 1, 0);
}
