// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#pragma once
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "gfx-source-texture.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"

// OBS
extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

#define S_SHADER_FILE "Shader.File"
#define S_SHADER_TECHNIQUE "Shader.Technique"

namespace gfx {
	namespace effect_source {
		enum class value_mode {
			INPUT,
			SLIDER,
		};
		enum class string_mode {
			TEXT,
			MULTILINE,
			PASSWORD,
		};
		enum class texture_mode {
			FILE,
			SOURCE,
		};

		class effect_source;

		class parameter {
			protected:
			std::weak_ptr<gfx::effect_source::effect_source> _parent;

			std::shared_ptr<gs::effect>           _effect;
			std::shared_ptr<gs::effect_parameter> _param;

			std::string _name;
			std::string _visible_name;
			std::string _description;
			std::string _formulae;
			bool        _visible;

			public:
			parameter(std::shared_ptr<gfx::effect_source::effect_source> parent, std::shared_ptr<gs::effect> effect,
					  std::shared_ptr<gs::effect_parameter> param);
			virtual ~parameter();

			virtual void defaults(obs_properties_t* props, obs_data_t* data) = 0;

			virtual void properties(obs_properties_t* props) = 0;

			virtual void remove_properties(obs_properties_t* props) = 0;

			virtual void update(obs_data_t* data) = 0;

			virtual void tick(float_t time) = 0;

			virtual void prepare() = 0;

			virtual void assign() = 0;

			std::shared_ptr<gs::effect_parameter> get_param();

			virtual void enum_active_sources(obs_source_enum_proc_t, void*){};

			public:
			static std::shared_ptr<gfx::effect_source::parameter>
				create(std::shared_ptr<gfx::effect_source::effect_source> parent, std::shared_ptr<gs::effect> effect,
					   std::shared_ptr<gs::effect_parameter> param);
		};

		class bool_parameter : public parameter {
			bool _value;

			public:
			bool_parameter(std::shared_ptr<gfx::effect_source::effect_source> parent,
						   std::shared_ptr<gs::effect> effect, std::shared_ptr<gs::effect_parameter> param);

			virtual void defaults(obs_properties_t* props, obs_data_t* data) override;

			virtual void properties(obs_properties_t* props) override;

			virtual void remove_properties(obs_properties_t* props) override;

			virtual void update(obs_data_t* data) override;

			virtual void tick(float_t time) override;

			virtual void prepare() override;

			virtual void assign() override;
		};

		class value_parameter : public parameter {
			union {
				float_t f[4];
				int32_t i[4];
			} _value;
			union {
				float_t f[4];
				int32_t i[4];
			} _minimum;
			union {
				float_t f[4];
				int32_t i[4];
			} _maximum;
			union {
				float_t f[4];
				int32_t i[4];
			} _step;
			value_mode _mode = value_mode::INPUT;

			struct {
				std::string name[4];
				std::string visible_name[4];
			} _cache;

			public:
			value_parameter(std::shared_ptr<gfx::effect_source::effect_source> parent,
							std::shared_ptr<gs::effect> effect, std::shared_ptr<gs::effect_parameter> param);

			virtual void defaults(obs_properties_t* props, obs_data_t* data) override;

			virtual void properties(obs_properties_t* props) override;

			virtual void remove_properties(obs_properties_t* props) override;

			virtual void update(obs_data_t* data) override;

			virtual void tick(float_t time) override;

			virtual void prepare() override;

			virtual void assign() override;
		};

		class matrix_parameter : public parameter {
			matrix4    _value;
			matrix4    _minimum;
			matrix4    _maximum;
			matrix4    _step;
			value_mode _mode = value_mode::INPUT;

			struct {
				std::string name[16];
				std::string visible_name[16];
			} _cache;

			public:
			matrix_parameter(std::shared_ptr<gfx::effect_source::effect_source> parent,
							 std::shared_ptr<gs::effect> effect, std::shared_ptr<gs::effect_parameter> param);

			virtual void defaults(obs_properties_t* props, obs_data_t* data) override;

			virtual void properties(obs_properties_t* props) override;

			virtual void remove_properties(obs_properties_t* props) override;

			virtual void update(obs_data_t* data) override;

			virtual void tick(float_t time) override;

			virtual void prepare() override;

			virtual void assign() override;
		};

		class string_parameter : public parameter {
			std::string _value;
			string_mode _mode = string_mode::TEXT;

			public:
			string_parameter(std::shared_ptr<gfx::effect_source::effect_source> parent,
							 std::shared_ptr<gs::effect> effect, std::shared_ptr<gs::effect_parameter> param);

			virtual void defaults(obs_properties_t* props, obs_data_t* data) override;

			virtual void properties(obs_properties_t* props) override;

			virtual void remove_properties(obs_properties_t* props) override;

			virtual void update(obs_data_t* data) override;

			virtual void tick(float_t time) override;

			virtual void prepare() override;

			virtual void assign() override;
		};

		class texture_parameter : public parameter {
			std::string                  _file_name;
			std::shared_ptr<gs::texture> _file;

			float_t _last_check;
			size_t  _last_size;
			time_t  _last_modify_time;
			time_t  _last_create_time;

			std::string                          _source_name;
			std::shared_ptr<obs::source>         _source;
			std::shared_ptr<gfx::source_texture> _source_renderer;
			std::shared_ptr<gs::texture>         _source_tex;

			texture_mode _mode = texture_mode::FILE;

			struct {
				std::string name[4];
				std::string visible_name[4];
			} _cache;

			void load_texture(std::string file);

			public:
			texture_parameter(std::shared_ptr<gfx::effect_source::effect_source> parent,
							  std::shared_ptr<gs::effect> effect, std::shared_ptr<gs::effect_parameter> param);

			bool modified2(obs_properties_t* props, obs_property_t* property, obs_data_t* settings);

			virtual void defaults(obs_properties_t* props, obs_data_t* data) override;

			virtual void properties(obs_properties_t* props) override;

			virtual void remove_properties(obs_properties_t* props) override;

			virtual void update(obs_data_t* data) override;

			virtual void tick(float_t time) override;

			virtual void prepare() override;

			virtual void assign() override;

			virtual void enum_active_sources(obs_source_enum_proc_t, void*) override;
		};

		typedef std::pair<gs::effect_parameter::type, std::string>               param_ident_t;
		typedef std::function<bool(std::shared_ptr<gs::effect_parameter> param)> valid_property_cb_t;
		typedef std::function<void(std::shared_ptr<gs::effect> effect)>          param_override_cb_t;

		class effect_source : public std::enable_shared_from_this<effect_source> {
			obs_source_t* _self;

			std::string                                         _file;
			std::shared_ptr<gs::effect>                         _effect;
			std::string                                         _tech;
			std::map<param_ident_t, std::shared_ptr<parameter>> _params;

			std::shared_ptr<gs::vertex_buffer> _tri;

			float_t _last_check;
			size_t  _last_size;
			time_t  _last_modify_time;
			time_t  _last_create_time;

			float_t _time;
			float_t _time_active;
			float_t _time_since_last_tick;

			std::uniform_real_distribution<float_t> _random_dist{0.f, 1.f};
			std::default_random_engine              _random_generator;

			valid_property_cb_t _cb_valid;
			param_override_cb_t _cb_override;

			bool modified2(obs_properties_t* props, obs_property_t* property, obs_data_t* settings);

			void load_file(std::string file);

			public:
			effect_source(obs_source_t* self);
			~effect_source();

			void properties(obs_properties_t* props);

			void update(obs_data_t* data);

			bool tick(float_t time);

			void render();

			obs_source_t* get_self();

			void enum_active_sources(obs_source_enum_proc_t, void*);

			public:
			void set_valid_property_cb(valid_property_cb_t cb);

			void set_override_cb(param_override_cb_t cb);
		};
	} // namespace effect_source
} // namespace gfx
