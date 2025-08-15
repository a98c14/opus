#include "os_file_win32.h"

internal OS_Handle
os_file_open(String path, OS_AccessFlags flags)
{
    OS_Handle result               = {0};
    ArenaTemp scratch              = scratch_begin(0, 0);
    String16  path16               = str16_from_8(scratch.arena, path);
    DWORD     access_flags         = 0;
    DWORD     share_mode           = 0;
    DWORD     creation_disposition = OPEN_EXISTING;
    if (flags & OS_AccessFlag_Read)
    {
        access_flags |= GENERIC_READ;
    }
    if (flags & OS_AccessFlag_Write)
    {
        access_flags |= GENERIC_WRITE;
    }
    if (flags & OS_AccessFlag_Execute)
    {
        access_flags |= GENERIC_EXECUTE;
    }
    if (flags & OS_AccessFlag_ShareRead)
    {
        share_mode |= FILE_SHARE_READ;
    }
    if (flags & OS_AccessFlag_ShareWrite)
    {
        share_mode |= FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }
    if (flags & OS_AccessFlag_Write)
    {
        creation_disposition = CREATE_ALWAYS;
    }
    if (flags & OS_AccessFlag_Append)
    {
        creation_disposition = OPEN_ALWAYS;
    }
    HANDLE file = CreateFileW((WCHAR*)path16.value, access_flags, share_mode, 0, creation_disposition, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE)
    {
        result.v = (uint64)file;
    }
    scratch_end(scratch);
    return result;
}

internal bool32
os_file_close(OS_Handle file)
{
    if (os_handle_match(file, os_handle_zero()))
        return 0;

    HANDLE handle = (HANDLE)file.v;
    BOOL   result = CloseHandle(handle);
    return result;
}

internal uint64
os_file_size(OS_Handle file)
{
    uint64 file_size;
    HANDLE handle = (HANDLE)file.v;
    GetFileSizeEx(handle, (LARGE_INTEGER*)&file_size);
    return file_size;
}

internal bool32
os_file_exists_at_path(String path)
{
    ArenaTemp scratch    = scratch_begin(0, 0);
    String16  path16     = str16_from_8(scratch.arena, path);
    DWORD     attributes = GetFileAttributesW((WCHAR*)path16.value);
    bool32    exists     = (attributes != INVALID_FILE_ATTRIBUTES) && !!(~attributes & FILE_ATTRIBUTE_DIRECTORY);
    scratch_end(scratch);
    return exists;
}

internal bool32
os_file_delete_at_path(String path)
{
    ArenaTemp scratch = scratch_begin(0, 0);
    String16  path16  = str16_from_8(scratch.arena, path);
    bool32    result  = DeleteFileW((WCHAR*)path16.value);
    scratch_end(scratch);
    return result;
}

internal uint64
os_file_read(OS_Handle file, uint64 offset, uint64 size, void* out_data)
{
    if (os_handle_match(file, os_handle_zero()))
        return 0;

    HANDLE handle    = (HANDLE)file.v;
    uint64 file_size = 0;
    GetFileSizeEx(handle, (LARGE_INTEGER*)&file_size);
    size   = clamp(0, size, file_size);
    offset = clamp(0, offset, file_size);

    uint64 total_read_size = 0;
    uint64 to_read         = size;
    for (uint64 current_offset = offset; total_read_size < to_read;)
    {
        uint64 remaining   = to_read - total_read_size;
        uint32 remaining32 = saturate_uint32_from_uint64(remaining);

        DWORD      read_size  = 0;
        OVERLAPPED overlapped = {0};
        overlapped.Offset     = (current_offset & 0x00000000ffffffffull);
        overlapped.OffsetHigh = (current_offset & 0xffffffff00000000ull) >> 32;
        ReadFile(handle, (uint8*)out_data + total_read_size, remaining32, &read_size, &overlapped);
        current_offset += read_size;
        total_read_size += read_size;
        if (read_size != remaining32)
            break;
    }

    return total_read_size;
}

internal uint64
os_file_write(OS_Handle file, uint64 offset, uint64 size, void* data)
{
    if (os_handle_match(file, os_handle_zero()))
        return 0;

    HANDLE wiNET_Handle         = (HANDLE)file.v;
    uint64 src_off              = 0;
    uint64 dst_off              = offset;
    uint64 bytes_to_write_total = size;
    uint64 total_bytes_written  = 0;
    for (; src_off < bytes_to_write_total;)
    {
        void*  bytes_src   = (void*)((uint8*)data + src_off);
        uint64 remaining   = (bytes_to_write_total - src_off);
        uint32 remaining32 = saturate_uint32_from_uint64(remaining);

        uint32     bytes_written = 0;
        OVERLAPPED overlapped    = {0};
        overlapped.Offset        = (dst_off & 0x00000000ffffffffull);
        overlapped.OffsetHigh    = (dst_off & 0xffffffff00000000ull) >> 32;
        BOOL success             = WriteFile(wiNET_Handle, bytes_src, remaining32, (DWORD*)&bytes_written, &overlapped);
        if (success == 0)
            break;
        src_off += bytes_written;
        dst_off += bytes_written;
        total_bytes_written += bytes_written;
    }
    return total_bytes_written;
}

internal String
os_file_get_working_directory(Arena* arena)
{
    uint32    max_path_size = 8192;
    ArenaTemp temp          = scratch_begin(&arena, 1);

    String16 path16 = {0};
    path16.value    = arena_push_array_zero(temp.arena, WCHAR, max_path_size);
    path16.size     = GetModuleFileNameW(0, (WCHAR*)path16.value, max_path_size - 1);
    String path     = str8_from_16(arena, path16);
    scratch_end(temp);
    return path;
}