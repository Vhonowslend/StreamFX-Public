// Copyright (c) 2021 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "gfx-lut.hpp"

#include <mutex>

#include "obs/gs/gs-helper.hpp"
#include "plugin.hpp"

using namespace streamfx;

#define LOCAL_PREFIX "<gfx::lut::data> "

std::shared_ptr<gfx::lut::data> gfx::lut::data::instance()
{
	static std::weak_ptr<gfx::lut::data> _instance;
	static std::mutex                    _mutex;

	std::lock_guard<std::mutex> lock(_mutex);

	auto reference = _instance.lock();
	if (!reference) {
		reference = std::shared_ptr<gfx::lut::data>(new gfx::lut::data());
		_instance = reference;
	}
	return reference;
}

gfx::lut::data::data() : _producer_effect(), _consumer_effect()
{
	auto gctx = streamfx::obs::gs::context();

	std::filesystem::path lut_producer_path = streamfx::data_file_path("effects/lut-producer.effect");
	if (std::filesystem::exists(lut_producer_path)) {
		try {
			_producer_effect = std::make_shared<streamfx::obs::gs::effect>(lut_producer_path);
		} catch (std::exception const& ex) {
			DLOG_ERROR(LOCAL_PREFIX "Loading LUT Producer effect failed: %s", ex.what());
		}
	}

	std::filesystem::path lut_consumer_path = streamfx::data_file_path("effects/lut-consumer.effect");
	if (std::filesystem::exists(lut_consumer_path)) {
		try {
			_consumer_effect = std::make_shared<streamfx::obs::gs::effect>(lut_consumer_path);
		} catch (std::exception const& ex) {
			DLOG_ERROR(LOCAL_PREFIX "Loading LUT Consumer effect failed: %s", ex.what());
		}
	}
}

gfx::lut::data::~data()
{
	auto gctx = streamfx::obs::gs::context();
	_producer_effect.reset();
	_consumer_effect.reset();
}
