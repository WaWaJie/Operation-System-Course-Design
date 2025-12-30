#pragma once
// 页面结构
struct Page {
    int page_id;        // 页面号
    int last_access;    // 最近访问时间（LRU用）
    int access_count;   // 访问次数（LFU用）
    bool in_memory;     // 是否在内存中
    bool is_dirty;      // 是否被修改过

    Page(int id) : page_id(id), last_access(0), access_count(0),
        in_memory(false), is_dirty(false) {
    }

    // 更新访问信息
    void update(int current_time) {
        last_access = current_time;
        access_count++;
    }

    // 比较函数用于排序
    bool operator<(const Page& other) const {
        return page_id < other.page_id;
    }
};

