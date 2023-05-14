#pragma once
#include "handler.hpp"

namespace streamfx::encoder::ffmpeg {
	class debug : public handler {
		public:
		debug();
		virtual ~debug(){};

		virtual void properties(ffmpeg_instance* instance, obs_properties_t* props);
	};
} // namespace streamfx::encoder::ffmpeg
