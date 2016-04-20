// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <benchmarks/core/utils/Memory.hpp>

#include <fstream>
#include <iostream>


#if defined(_WIN32)
#	include <windows.h>
#	include <psapi.h>
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#	include <unistd.h>
#	include <sys/resource.h>
#	if defined(__APPLE__) && defined(__MACH__)
#		include <mach/mach.h>
#		include <mach/message.h>
#		include <mach/mach_host.h>
#		include <mach/host_info.h>
#	endif
#endif


namespace benchmarks
{

	int64_t Memory::GetRss()
	{
#if defined(_WIN32)

		PROCESS_MEMORY_COUNTERS info;
		if (!GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)))
			return 0;
		return info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)

		struct mach_task_basic_info info;
		mach_msg_type_number_t info_count = MACH_TASK_BASIC_INFO_COUNT;
		if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &info_count) != KERN_SUCCESS)
			return 0;
		return info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)

		std::ifstream f("/proc/self/statm");
		int64_t size, rss;
		f >> size >> rss;
		return f.bad() ? 0 : (rss * sysconf( _SC_PAGESIZE));

#else

		return 0;

#endif
	}


	int64_t Memory::GetTotalPhys()
	{
#if defined(_WIN32)

		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		if (!GlobalMemoryStatusEx(&status))
			return 0;
		return status.ullTotalPhys;

#elif defined(__APPLE__) && defined(__MACH__)

		int mib [] = { CTL_HW, HW_MEMSIZE };
		int64_t value = 0;
		size_t length = sizeof(value);
		if(sysctl(mib, 2, &value, &length, NULL, 0) != 0)
			return 0;
		return value;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)

		int64_t pages = sysconf(_SC_PHYS_PAGES);
		int64_t page_size = sysconf(_SC_PAGE_SIZE);
		return pages * page_size;

#else

		return 0;

#endif
	}


	int64_t Memory::GetAvailablePhys()
	{
#if defined(_WIN32)

		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		if (!GlobalMemoryStatusEx(&status))
			return 0;
		return status.ullAvailPhys;

#elif defined(__APPLE__) && defined(__MACH__)

		mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
		vm_statistics_data_t vmstat;
		if(host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) != KERN_SUCCESS)
			return 0;
		return int64_t(vmstat.pagesize) * int64_t(vmstat.free_count);

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)

		int64_t pages = sysconf(_SC_AVPHYS_PAGES);
		int64_t page_size = sysconf(_SC_PAGE_SIZE);
		return pages * page_size;

#else

		return 0;

#endif
	}

}
