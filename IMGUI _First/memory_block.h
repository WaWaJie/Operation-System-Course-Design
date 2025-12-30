#pragma once

#include<string>
#include<vector>

struct MemoryBlock
{
	int start_address; // 起始地址
	int size;          // 大小
	bool is_free;     // 是否空闲
	bool need_recycle = false; // 是否需要回收
	std::string bname = "Empty"; // 块名
	std::string pname="Empty"; // 所属进程名
	MemoryBlock(int start = 0, int sz = 0, bool free = true)
		: start_address(start), size(sz), is_free(free) {
	}
};


