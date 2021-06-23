/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "util-profiler.hpp"
#include <iterator>

streamfx::util::profiler::profiler() {}

streamfx::util::profiler::~profiler() {}

std::shared_ptr<streamfx::util::profiler::instance> streamfx::util::profiler::track()
{
	return std::make_shared<streamfx::util::profiler::instance>(shared_from_this());
}

void streamfx::util::profiler::track(std::chrono::nanoseconds duration)
{
	std::unique_lock<std::mutex> ul(_timings_lock);
	auto                         itr = _timings.find(duration);
	if (itr == _timings.end()) {
		_timings.emplace(duration, 1u);
	} else {
		itr->second++;
	}
}

uint64_t streamfx::util::profiler::count()
{
	uint64_t count = 0;

	std::map<std::chrono::nanoseconds, size_t> copy_timings;
	{
		std::unique_lock<std::mutex> ul(_timings_lock);
		copy(_timings.begin(), _timings.end(), std::inserter(copy_timings, copy_timings.end()));
	}

	for (auto kv : copy_timings) {
		count += kv.second;
	}

	return count;
}

std::chrono::nanoseconds streamfx::util::profiler::total_duration()
{
	std::chrono::nanoseconds duration{0};

	std::map<std::chrono::nanoseconds, size_t> copy_timings;
	{
		std::unique_lock<std::mutex> ul(_timings_lock);
		copy(_timings.begin(), _timings.end(), std::inserter(copy_timings, copy_timings.end()));
	}

	for (auto kv : copy_timings) {
		duration += kv.first * kv.second;
	}

	return duration;
}

double_t streamfx::util::profiler::average_duration()
{
	std::chrono::nanoseconds duration{0};
	uint64_t                 count = 0;

	std::map<std::chrono::nanoseconds, size_t> copy_timings;
	{
		std::unique_lock<std::mutex> ul(_timings_lock);
		copy(_timings.begin(), _timings.end(), std::inserter(copy_timings, copy_timings.end()));
	}

	for (auto kv : copy_timings) {
		duration += kv.first * kv.second;
		count += kv.second;
	}

	return double_t(duration.count()) / double_t(count);
}

template<typename T>
inline bool is_equal(T a, T b, T c)
{
	return (a == b) || ((a >= (b - c)) && (a <= (b + c)));
}

std::chrono::nanoseconds streamfx::util::profiler::percentile(double_t percentile, bool by_time)
{
	constexpr double_t edge  = 0.00005;
	uint64_t           calls = count();

	std::map<std::chrono::nanoseconds, size_t> copy_timings;
	{
		std::unique_lock<std::mutex> ul(_timings_lock);
		copy(_timings.begin(), _timings.end(), inserter(copy_timings, copy_timings.end()));
	}
	if (by_time) { // Return by time percentile.
		// Find largest and smallest time.
		std::chrono::nanoseconds smallest = copy_timings.begin()->first;
		std::chrono::nanoseconds largest  = copy_timings.rbegin()->first;
		std::chrono::nanoseconds variance = largest - smallest;

		for (auto kv : copy_timings) {
			double_t kv_pct = double_t((kv.first - smallest).count()) / double_t(variance.count());
			if (is_equal<double_t>(kv_pct, percentile, edge) || (kv_pct > percentile)) {
				return std::chrono::nanoseconds(kv.first);
			}
		}
	} else { // Return by call percentile.
		if (percentile == 0.0) {
			return copy_timings.begin()->first;
		}

		uint64_t accu_calls_now = 0;
		for (auto kv : copy_timings) {
			uint64_t accu_calls_last = accu_calls_now;
			accu_calls_now += kv.second;

			double_t percentile_last = double_t(accu_calls_last) / double_t(calls);
			double_t percentile_now  = double_t(accu_calls_now) / double_t(calls);

			if (is_equal<double_t>(percentile, percentile_now, edge)
				|| ((percentile_last < percentile) && (percentile_now > percentile))) {
				return std::chrono::nanoseconds(kv.first);
			}
		}
	}

	return std::chrono::nanoseconds(-1);
}

streamfx::util::profiler::instance::instance(std::shared_ptr<streamfx::util::profiler> parent)
	: _parent(parent), _start(std::chrono::high_resolution_clock::now())
{}

streamfx::util::profiler::instance::~instance()
{
	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - _start;
	if (_parent) {
		_parent->track(dur);
	}
}

void streamfx::util::profiler::instance::cancel()
{
	_parent.reset();
}

void streamfx::util::profiler::instance::reparent(std::shared_ptr<streamfx::util::profiler> parent)
{
	_parent = parent;
}
