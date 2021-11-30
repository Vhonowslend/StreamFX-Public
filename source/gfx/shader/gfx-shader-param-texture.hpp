
#pragma once
#include <obs.h>
#include <chrono>
#include <mutex>
#include "gfx-shader-param.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/obs-tools.hpp"

namespace streamfx::gfx {
	namespace shader {
		enum class texture_field_type {
			Input,
			Enum,
		};

		texture_field_type get_texture_field_type_from_string(std::string v);

		enum class texture_type {
			File,
			Source,
		};

		struct texture_data {
			std::filesystem::path file;
		};

		struct texture_enum_data {
			std::string  name;
			texture_data data;
		};

		struct texture_parameter : public parameter {
			// Descriptor
			texture_field_type       _field_type;
			std::vector<std::string> _keys;

			// Enumeration Information
			std::list<texture_enum_data> _values;

			// Data
			texture_type          _type;
			bool                  _active;
			bool                  _visible;
			std::filesystem::path _default;

			// Data: Dirty state
			bool                                           _dirty;
			std::chrono::high_resolution_clock::time_point _dirty_ts;

			// Data: File
			std::filesystem::path                       _file_path;
			std::shared_ptr<streamfx::obs::gs::texture> _file_texture;

			// Data: Source
			std::string                                           _source_name;
			std::shared_ptr<obs_source_t>                         _source;
			std::shared_ptr<streamfx::obs::tools::child_source>   _source_child;
			std::shared_ptr<streamfx::obs::tools::active_source>  _source_active;
			std::shared_ptr<streamfx::obs::tools::visible_source> _source_visible;
			std::shared_ptr<streamfx::obs::gs::rendertarget>      _source_rendertarget;

			public:
			texture_parameter(streamfx::gfx::shader::shader* parent, streamfx::obs::gs::effect_parameter param,
							  std::string prefix);
			virtual ~texture_parameter();

			void defaults(obs_data_t* settings) override;

			static bool modified_type(void*, obs_properties_t*, obs_property_t*, obs_data_t*);

			void properties(obs_properties_t* props, obs_data_t* settings) override;

			void update(obs_data_t* settings) override;

			void assign() override;

			void visible(bool visible) override;

			void active(bool enabled) override;

			public:
			inline texture_field_type field_type()
			{
				return _field_type;
			}
		};
	} // namespace shader
} // namespace streamfx::gfx
