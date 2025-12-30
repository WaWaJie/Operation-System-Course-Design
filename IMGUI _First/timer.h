#ifndef _TIMER_H_
#define _TIMER_H_

#include <functional>
#include<cmath>
#include<iostream>

class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	void restart()
	{
		pass_time = 0;
		shotted = false;
	}

	void set_wait_time(float val)
	{
		wait_time = val;
	}

	void set_one_shot(bool flag)
	{
		one_shot = flag;
	}

	void set_on_timeout(std::function<void()> on_timeout)
	{
		this->on_timeout = on_timeout;
	}

	void pause()
	{
		paused = true;
	}

	void resume()
	{
		paused = false;
	}

	void on_update(float delta)
	{
		if (paused) return;

		pass_time += delta;
		if (pass_time >= wait_time)
		{
			bool can_shot = (!one_shot || (one_shot && !shotted));
			shotted = true;
			if (can_shot && on_timeout)
				on_timeout();
			pass_time -= wait_time;
		}
	}

	// 检查定时器是否已经超时
	bool is_time_out() const
	{
		if (one_shot)
		{
			// 对于一次性定时器，超时后shotted会被设置为true
			return shotted;
		}
		else
		{
			// 对于重复定时器，只要pass_time >= wait_time就表示当前周期超时
			return pass_time >= wait_time;
		}
	}

	//获取剩余时间
	float get_remain_time()
	{
		return std::max(wait_time - pass_time, (float)0.0);
	}

private:
	float pass_time = 0;
	float wait_time = 0;
	bool paused = false;
	bool shotted = false;
	bool one_shot = false;
	std::function<void()> on_timeout;

};

#endif // !_TIMER_H_