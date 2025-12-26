#pragma once

#include<string>
#include<vector>

struct ProcessResource
{
	std::string name = "";
	int id = 1;
	int need_num = 0;

	ProcessResource() = default;
	ProcessResource(std::string name, int id, int need_num) :
		name(name), id(id), need_num(need_num){ }

	bool operator<(const std::shared_ptr<ProcessResource>& other) const
	{
		return id < other.get()->id;
	}

};
