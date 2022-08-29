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

//--------------------------------------------------------------------------------//
// THIS FEATURE IS DEPRECATED. SUBMITTED PATCHES WILL BE REJECTED.
//--------------------------------------------------------------------------------//

#include "encoder-aom-av1.hpp"
#include "util/util-logging.hpp"

#include "warning-disable.hpp"
#include <filesystem>
#include <thread>
#include "warning-enable.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<encoder::aom::av1> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#define ST_I18N "Encoder.AOM.AV1"

// Preset
#define ST_I18N_DEPRECATED ST_I18N ".Deprecated"
#define ST_I18N_ENCODER ST_I18N ".Encoder"
#define ST_I18N_ENCODER_USAGE ST_I18N_ENCODER ".Usage"
#define ST_I18N_ENCODER_USAGE_GOODQUALITY ST_I18N_ENCODER_USAGE ".GoodQuality"
#define ST_I18N_ENCODER_USAGE_REALTIME ST_I18N_ENCODER_USAGE ".RealTime"
#define ST_I18N_ENCODER_USAGE_ALLINTRA ST_I18N_ENCODER_USAGE ".AllIntra"
#define ST_KEY_ENCODER_USAGE "Encoder.Usage"
#define ST_I18N_ENCODER_CPUUSAGE ST_I18N_ENCODER ".CPUUsage"
#define ST_I18N_ENCODER_CPUUSAGE_0 ST_I18N_ENCODER ".CPUUsage.0"
#define ST_I18N_ENCODER_CPUUSAGE_1 ST_I18N_ENCODER ".CPUUsage.1"
#define ST_I18N_ENCODER_CPUUSAGE_2 ST_I18N_ENCODER ".CPUUsage.2"
#define ST_I18N_ENCODER_CPUUSAGE_3 ST_I18N_ENCODER ".CPUUsage.3"
#define ST_I18N_ENCODER_CPUUSAGE_4 ST_I18N_ENCODER ".CPUUsage.4"
#define ST_I18N_ENCODER_CPUUSAGE_5 ST_I18N_ENCODER ".CPUUsage.5"
#define ST_I18N_ENCODER_CPUUSAGE_6 ST_I18N_ENCODER ".CPUUsage.6"
#define ST_I18N_ENCODER_CPUUSAGE_7 ST_I18N_ENCODER ".CPUUsage.7"
#define ST_I18N_ENCODER_CPUUSAGE_8 ST_I18N_ENCODER ".CPUUsage.8"
#define ST_I18N_ENCODER_CPUUSAGE_9 ST_I18N_ENCODER ".CPUUsage.9"
#define ST_I18N_ENCODER_CPUUSAGE_10 ST_I18N_ENCODER ".CPUUsage.10"
#define ST_KEY_ENCODER_CPUUSAGE "Encoder.CPUUsage"
#define ST_KEY_ENCODER_PROFILE "Encoder.Profile"

// Rate Control
#define ST_I18N_RATECONTROL ST_I18N ".RateControl"
#define ST_I18N_RATECONTROL_MODE ST_I18N_RATECONTROL ".Mode"
#define ST_I18N_RATECONTROL_MODE_CBR ST_I18N_RATECONTROL_MODE ".CBR"
#define ST_I18N_RATECONTROL_MODE_VBR ST_I18N_RATECONTROL_MODE ".VBR"
#define ST_I18N_RATECONTROL_MODE_CQ ST_I18N_RATECONTROL_MODE ".CQ"
#define ST_I18N_RATECONTROL_MODE_Q ST_I18N_RATECONTROL_MODE ".Q"
#define ST_KEY_RATECONTROL_MODE "RateControl.Mode"
#define ST_I18N_RATECONTROL_LOOKAHEAD ST_I18N_RATECONTROL ".LookAhead"
#define ST_KEY_RATECONTROL_LOOKAHEAD "RateControl.LookAhead"
#define ST_I18N_RATECONTROL_LIMITS ST_I18N_RATECONTROL ".Limits"
#define ST_I18N_RATECONTROL_LIMITS_BITRATE ST_I18N_RATECONTROL_LIMITS ".Bitrate"
#define ST_KEY_RATECONTROL_LIMITS_BITRATE "RateControl.Limits.Bitrate"
#define ST_I18N_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT ST_I18N_RATECONTROL_LIMITS_BITRATE ".Undershoot"
#define ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT "RateControl.Limits.Bitrate.Undershoot"
#define ST_I18N_RATECONTROL_LIMITS_BITRATE_OVERSHOOT ST_I18N_RATECONTROL_LIMITS_BITRATE ".Overshoot"
#define ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT "RateControl.Limits.Bitrate.Overshoot"
#define ST_I18N_RATECONTROL_LIMITS_QUALITY ST_I18N_RATECONTROL_LIMITS ".Quality"
#define ST_KEY_RATECONTROL_LIMITS_QUALITY "RateControl.Limits.Quality"
#define ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MINIMUM ST_I18N_RATECONTROL_LIMITS ".Quantizer.Minimum"
#define ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM "RateControl.Limits.Quantizer.Minimum"
#define ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM ST_I18N_RATECONTROL_LIMITS ".Quantizer.Maximum"
#define ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM "RateControl.Limits.Quantizer.Maximum"
#define ST_I18N_RATECONTROL_BUFFER ST_I18N_RATECONTROL ".Buffer"
#define ST_I18N_RATECONTROL_BUFFER_SIZE ST_I18N_RATECONTROL_BUFFER ".Size"
#define ST_KEY_RATECONTROL_BUFFER_SIZE "RateControl.Buffer.Size"
#define ST_I18N_RATECONTROL_BUFFER_SIZE_INITIAL ST_I18N_RATECONTROL_BUFFER_SIZE ".Initial"
#define ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL "RateControl.Buffer.Size.Initial"
#define ST_I18N_RATECONTROL_BUFFER_SIZE_OPTIMAL ST_I18N_RATECONTROL_BUFFER_SIZE ".Optimal"
#define ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL "RateControl.Buffer.Size.Optimal"

// Key-Frames
#define ST_I18N_KEYFRAMES ST_I18N ".KeyFrames"
#define ST_I18N_KEYFRAMES_INTERVALTYPE ST_I18N_KEYFRAMES ".IntervalType"
#define ST_I18N_KEYFRAMES_INTERVALTYPE_SECONDS ST_I18N_KEYFRAMES_INTERVALTYPE ".Seconds"
#define ST_I18N_KEYFRAMES_INTERVALTYPE_FRAMES ST_I18N_KEYFRAMES_INTERVALTYPE ".Frames"
#define ST_KEY_KEYFRAMES_INTERVALTYPE "KeyFrames.IntervalType"
#define ST_I18N_KEYFRAMES_INTERVAL ST_I18N_KEYFRAMES ".Interval"
#define ST_KEY_KEYFRAMES_INTERVAL_SECONDS "KeyFrames.Interval.Seconds"
#define ST_KEY_KEYFRAMES_INTERVAL_FRAMES "KeyFrames.Interval.Frames"

// Advanced
#define ST_I18N_ADVANCED ST_I18N ".Advanced"
#define ST_I18N_ADVANCED_THREADS ST_I18N_ADVANCED ".Threads"
#define ST_KEY_ADVANCED_THREADS "Advanced.Threads"
#define ST_I18N_ADVANCED_ROWMULTITHREADING ST_I18N_ADVANCED ".RowMultiThreading"
#define ST_KEY_ADVANCED_ROWMULTITHREADING "Advanced.RowMultiThreading"
#define ST_I18N_ADVANCED_TILE_COLUMNS ST_I18N_ADVANCED ".Tile.Columns"
#define ST_KEY_ADVANCED_TILE_COLUMNS "Advanced.Tile.Columns"
#define ST_I18N_ADVANCED_TILE_ROWS ST_I18N_ADVANCED ".Tile.Rows"
#define ST_KEY_ADVANCED_TILE_ROWS "Advanced.Tile.Rows"
#define ST_I18N_ADVANCED_TUNE ST_I18N_ADVANCED ".Tune"
#define ST_I18N_ADVANCED_TUNE_METRIC ST_I18N_ADVANCED_TUNE ".Metric"
#define ST_I18N_ADVANCED_TUNE_METRIC_PSNR ST_I18N_ADVANCED_TUNE_METRIC ".PSNR"
#define ST_I18N_ADVANCED_TUNE_METRIC_SSIM ST_I18N_ADVANCED_TUNE_METRIC ".SSIM"
#define ST_I18N_ADVANCED_TUNE_METRIC_VMAFWITHPREPROCESSING ST_I18N_ADVANCED_TUNE_METRIC ".VMAF.WithPreprocessing"
#define ST_I18N_ADVANCED_TUNE_METRIC_VMAFWITHOUTPREPROCESSING ST_I18N_ADVANCED_TUNE_METRIC ".VMAF.WithoutPreProcessing"
#define ST_I18N_ADVANCED_TUNE_METRIC_VMAFMAXGAIN ST_I18N_ADVANCED_TUNE_METRIC ".VMAF.MaxGain"
#define ST_I18N_ADVANCED_TUNE_METRIC_VMAFNEGMAXGAIN ST_I18N_ADVANCED_TUNE_METRIC ".VMAF.NegMaxGain"
#define ST_I18N_ADVANCED_TUNE_METRIC_BUTTERAUGLI ST_I18N_ADVANCED_TUNE_METRIC ".Butteraugli"
#define ST_KEY_ADVANCED_TUNE_METRIC "Advanced.Tune.Metric"
#define ST_I18N_ADVANCED_TUNE_CONTENT ST_I18N_ADVANCED_TUNE ".Content"
#define ST_I18N_ADVANCED_TUNE_CONTENT_SCREEN ST_I18N_ADVANCED_TUNE_CONTENT ".Screen"
#define ST_I18N_ADVANCED_TUNE_CONTENT_FILM ST_I18N_ADVANCED_TUNE_CONTENT ".Film"
#define ST_KEY_ADVANCED_TUNE_CONTENT "Advanced.Tune.Content"

using namespace streamfx::encoder::aom::av1;

static constexpr std::string_view HELP_URL = "https://github.com/Xaymar/obs-StreamFX/wiki/Encoder-AOM-AV1";

const char* obs_video_format_to_string(video_format format)
{
	switch (format) {
	case VIDEO_FORMAT_I420:
		return "I420";
	case VIDEO_FORMAT_NV12:
		return "NV12";
	case VIDEO_FORMAT_YVYU:
		return "YVYU";
	case VIDEO_FORMAT_YUY2:
		return "YUY";
	case VIDEO_FORMAT_UYVY:
		return "UYVY";
	case VIDEO_FORMAT_RGBA:
		return "RGBA";
	case VIDEO_FORMAT_BGRA:
		return "BGRA";
	case VIDEO_FORMAT_BGRX:
		return "BGRX";
	case VIDEO_FORMAT_Y800:
		return "Y800";
	case VIDEO_FORMAT_I444:
		return "I444";
	case VIDEO_FORMAT_BGR3:
		return "BGR3";
	case VIDEO_FORMAT_I422:
		return "I422";
	case VIDEO_FORMAT_I40A:
		return "I40A";
	case VIDEO_FORMAT_I42A:
		return "I42A";
	case VIDEO_FORMAT_YUVA:
		return "YUVA";
	case VIDEO_FORMAT_AYUV:
		return "AYUV";
	default:
		return "Unknown";
	}
}

const char* aom_color_format_to_string(aom_img_fmt format)
{
	switch (format) {
	case AOM_IMG_FMT_AOMYV12:
		return "AOM-YV12";
	case AOM_IMG_FMT_AOMI420:
		return "AOM-I420";
	case AOM_IMG_FMT_YV12:
		return "YV12";
	case AOM_IMG_FMT_I420:
		return "I420";
	case AOM_IMG_FMT_I422:
		return "I422";
	case AOM_IMG_FMT_I444:
		return "I444";
	case AOM_IMG_FMT_YV1216:
		return "YV12-16";
	case AOM_IMG_FMT_I42016:
		return "I420-16";
	case AOM_IMG_FMT_I42216:
		return "I422-16";
	case AOM_IMG_FMT_I44416:
		return "I444-16";
	default:
		return "Unknown";
	}
}

const char* aom_color_trc_to_string(aom_transfer_characteristics_t trc)
{
	switch (trc) {
	case AOM_CICP_TC_BT_709:
		return "Bt.709";
	case AOM_CICP_TC_BT_470_M:
		return "Bt.407 M";
	case AOM_CICP_TC_BT_470_B_G:
		return "Bt.407 B/G";
	case AOM_CICP_TC_BT_601:
		return "Bt.601";
	case AOM_CICP_TC_SMPTE_240:
		return "SMPTE 240 M";
	case AOM_CICP_TC_LINEAR:
		return "Linear";
	case AOM_CICP_TC_LOG_100:
		return "Logarithmic (100:1 range)";
	case AOM_CICP_TC_LOG_100_SQRT10:
		return "Logarithmic (100*sqrt(10):1 range)";
	case AOM_CICP_TC_IEC_61966:
		return "IEC 61966-2-4";
	case AOM_CICP_TC_BT_1361:
		return "Bt.1361";
	case AOM_CICP_TC_SRGB:
		return "sRGB";
	case AOM_CICP_TC_BT_2020_10_BIT:
		return "Bt.2020 10b";
	case AOM_CICP_TC_BT_2020_12_BIT:
		return "Bt.2020 12b";
	case AOM_CICP_TC_SMPTE_2084:
		return "Bt.2100 PQ";
	case AOM_CICP_TC_SMPTE_428:
		return "SMPTE ST 428";
	case AOM_CICP_TC_HLG:
		return "Bt.2100 HLG";
	default:
		return "Unknown";
	}
}

const char* aom_rc_mode_to_string(aom_rc_mode mode)
{
	switch (mode) {
	case AOM_VBR:
		return "Variable Bitrate (VBR)";
	case AOM_CBR:
		return "Constant Bitrate (CBR)";
	case AOM_CQ:
		return "Constrained Quality (CQ)";
	case AOM_Q:
		return "Constant Quality (Q)";
	default:
		return "Unknown";
	}
}

const char* aom_kf_mode_to_string(aom_kf_mode mode)
{
	switch (mode) {
	case AOM_KF_AUTO:
		return "Automatic";
	case AOM_KF_DISABLED:
		return "Disabled";
	default:
		return "Unknown";
	}
}

const char* aom_tune_metric_to_string(aom_tune_metric tune)
{
	switch (tune) {
	case AOM_TUNE_PSNR:
		return "PSNR";
	case AOM_TUNE_SSIM:
		return "SSIM";
	case AOM_TUNE_VMAF_WITH_PREPROCESSING:
		return "VMAF w/ pre-processing";
	case AOM_TUNE_VMAF_WITHOUT_PREPROCESSING:
		return "VMAF w/o pre-processing";
	case AOM_TUNE_VMAF_MAX_GAIN:
		return "VMAF max. gain";
	case AOM_TUNE_VMAF_NEG_MAX_GAIN:
		return "VMAF negative max. gain";
	case AOM_TUNE_BUTTERAUGLI:
		return "Butteraugli";
	default:
		return "Unknown";
	}
}

const char* aom_tune_content_to_string(aom_tune_content tune)
{
	switch (tune) {
	case AOM_CONTENT_DEFAULT:
		return "Default";
	case AOM_CONTENT_FILM:
		return "Film";
	case AOM_CONTENT_SCREEN:
		return "Screen";
	default:
		return "Unknown";
	}
}

aom_av1_instance::aom_av1_instance(obs_data_t* settings, obs_encoder_t* self, bool is_hw)
	: obs::encoder_instance(settings, self, is_hw), _factory(aom_av1_factory::get()), _iface(nullptr), _ctx(), _cfg(),
	  _image_index(0), _images(), _global_headers(nullptr), _initialized(false), _settings()
{
	if (is_hw) {
		throw std::runtime_error("Hardware encoding isn't even registered, how did you get here?");
	}

	// Retrieve encoder interface.
	_iface = _factory->libaom_codec_av1_cx();
	if (!_iface) {
		throw std::runtime_error("AOM library does not provide AV1 encoder.");
	}

#ifdef ENABLE_PROFILING
	// Profilers
	_profiler_copy   = streamfx::util::profiler::create();
	_profiler_encode = streamfx::util::profiler::create();
	_profiler_packet = streamfx::util::profiler::create();
#endif

	{     // Generate Static Configuration
		{ // OBS Information
			video_scale_info                ovsi;
			video_t*                        video      = obs_encoder_video(_self);
			const struct video_output_info* video_info = video_output_get_info(video);

			ovsi.colorspace = video_info->colorspace;
			ovsi.format     = video_info->format;
			ovsi.range      = video_info->range;
			get_video_info(&ovsi);

			// Video
			_settings.width   = static_cast<uint16_t>(obs_encoder_get_width(_self));
			_settings.height  = static_cast<uint16_t>(obs_encoder_get_height(_self));
			_settings.fps.num = static_cast<uint32_t>(video_info->fps_num);
			_settings.fps.den = static_cast<uint32_t>(video_info->fps_den);

			// Color Format
			switch (ovsi.format) {
			case VIDEO_FORMAT_I420:
				_settings.color_format = AOM_IMG_FMT_I420;
				break;
			case VIDEO_FORMAT_I422:
				_settings.color_format = AOM_IMG_FMT_I422;
				break;
			case VIDEO_FORMAT_I444:
				_settings.color_format = AOM_IMG_FMT_I444;
				break;
			default:
				throw std::runtime_error("Color Format is unknown.");
			}

			// Color Space
			switch (ovsi.colorspace) {
			case VIDEO_CS_601:
				_settings.color_primaries = AOM_CICP_CP_BT_601;
				_settings.color_trc       = AOM_CICP_TC_BT_601;
				_settings.color_matrix    = AOM_CICP_MC_BT_601;
				break;
			case VIDEO_CS_709:
				_settings.color_primaries = AOM_CICP_CP_BT_709;
				_settings.color_trc       = AOM_CICP_TC_BT_709;
				_settings.color_matrix    = AOM_CICP_MC_BT_709;
				break;
			case VIDEO_CS_SRGB:
				_settings.color_primaries = AOM_CICP_CP_BT_709;
				_settings.color_trc       = AOM_CICP_TC_SRGB;
				_settings.color_matrix    = AOM_CICP_MC_BT_709;
				break;
			default:
				throw std::runtime_error("Color Space is unknown.");
			}

			// Color Range
			switch (ovsi.range) {
			case VIDEO_RANGE_FULL:
				_settings.color_range = AOM_CR_FULL_RANGE;
				break;
			case VIDEO_RANGE_PARTIAL:
				_settings.color_range = AOM_CR_STUDIO_RANGE;
				break;
			default:
				throw std::runtime_error("Color Range is unknown.");
			}

			// Monochrome
			_settings.monochrome = (video_info->format == VIDEO_FORMAT_Y800);
		}

		{ // Encoder
			_settings.profile = static_cast<codec::av1::profile>(obs_data_get_int(settings, ST_KEY_ENCODER_PROFILE));
			if (_settings.profile == codec::av1::profile::UNKNOWN) {
				// Resolve the automatic profile to a proper value.
				bool need_professional = (_settings.color_format == AOM_IMG_FMT_I422);
				bool need_high         = (_settings.color_format == AOM_IMG_FMT_I444) || _settings.monochrome;

				if (need_professional) {
					_settings.profile = codec::av1::profile::PROFESSIONAL;
				} else if (need_high) {
					_settings.profile = codec::av1::profile::HIGH;
				} else {
					_settings.profile = codec::av1::profile::MAIN;
				}
			}
		}

		{ // Rate Control
			_settings.rc_mode      = static_cast<aom_rc_mode>(obs_data_get_int(settings, ST_KEY_RATECONTROL_MODE));
			_settings.rc_lookahead = static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LOOKAHEAD));
		}

		{ // Threading
			if (auto threads = obs_data_get_int(settings, ST_KEY_ADVANCED_THREADS); threads > 0) {
				_settings.threads = static_cast<int8_t>(threads);
			} else {
				_settings.threads = static_cast<int8_t>(std::thread::hardware_concurrency());
			}
			_settings.rowmultithreading =
				static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_ADVANCED_ROWMULTITHREADING));
		}

		{ // Tiling
			_settings.tile_columns = static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_ADVANCED_TILE_COLUMNS));
			_settings.tile_rows    = static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_ADVANCED_TILE_ROWS));
		}

		{ // Tuning
			if (auto v = obs_data_get_int(settings, ST_KEY_ADVANCED_TUNE_METRIC); v != -1) {
				_settings.tune_metric = static_cast<aom_tune_metric>(v);
			}
			_settings.tune_content =
				static_cast<aom_tune_content>(obs_data_get_int(settings, ST_KEY_ADVANCED_TUNE_CONTENT));
		}
	}

	// Apply Settings
	update(settings);

	// Initialize Encoder
	if (auto error = _factory->libaom_codec_enc_init_ver(&_ctx, _iface, &_cfg, 0, AOM_ENCODER_ABI_VERSION);
		error != AOM_CODEC_OK) {
		const char* errstr = _factory->libaom_codec_err_to_string(error);
		D_LOG_ERROR("Failed to initialize codec, unexpected error: %s (code %" PRIu32 ")", errstr, error);
		throw std::runtime_error(errstr);
	}

	{ // Apply Static Control Settings

		{ // Color Information
#ifdef AOM_CTRL_AV1E_SET_COLOR_PRIMARIES
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_COLOR_PRIMARIES, _settings.color_primaries);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
							  "AV1E_SET_COLOR_PRIMARIES",                             //
							  (errstr ? errstr : ""), error,                          //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
				);
			}
#else
			D_LOG_ERROR("AOM library was built without AV1E_SET_COLOR_PRIMARIES, behavior is unknown.");
#endif

#ifdef AOM_CTRL_AV1E_SET_TRANSFER_CHARACTERISTICS
			if (auto error =
					_factory->libaom_codec_control(&_ctx, AV1E_SET_TRANSFER_CHARACTERISTICS, _settings.color_trc);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
							  "AV1E_SET_TRANSFER_CHARACTERISTICS",                    //
							  (errstr ? errstr : ""), error,                          //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
				);
			}
#else
			D_LOG_ERROR("AOM library was built without AV1E_SET_TRANSFER_CHARACTERISTICS, behavior is unknown.");
#endif

#ifdef AOM_CTRL_AV1E_SET_MATRIX_COEFFICIENTS
			if (auto error =
					_factory->libaom_codec_control(&_ctx, AV1E_SET_MATRIX_COEFFICIENTS, _settings.color_matrix);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
							  "AV1E_SET_MATRIX_COEFFICIENTS",                         //
							  (errstr ? errstr : ""), error,                          //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
				);
			}
#else
			D_LOG_ERROR("AOM library was built without AV1E_SET_MATRIX_COEFFICIENTS, behavior is unknown.");
#endif

#ifdef AOM_CTRL_AV1E_SET_COLOR_RANGE
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_COLOR_RANGE, _settings.color_range);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
							  "AV1E_SET_COLOR_RANGE",                                 //
							  (errstr ? errstr : ""), error,                          //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
				);
			}
#else
			D_LOG_ERROR("AOM library was built without AV1_SET_COLOR_RANGE, behavior is unknown.");
#endif

#ifdef AOM_CTRL_AV1E_SET_CHROMA_SAMPLE_POSITION
			// !TODO: Consider making this user-controlled. At the moment, this follows the H.264 chroma standard.
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_CHROMA_SAMPLE_POSITION, AOM_CSP_VERTICAL);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
							  "AV1E_SET_CHROMA_SAMPLE_POSITION",                      //
							  (errstr ? errstr : ""), error,                          //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
				);
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_RENDER_SIZE
			int32_t size[2] = {_settings.width, _settings.height};
			if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_RENDER_SIZE, &size);
				error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
							  "AV1E_SET_RENDER_SIZE",                                 //
							  (errstr ? errstr : ""), error,                          //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
				);
			}
#endif
		}

		// Apply Dynamic Control Settings
		if (!update(settings)) {
			throw std::runtime_error("Unexpected error during configuration.");
		}
	}

	// Preallocate global headers.
	_global_headers = _factory->libaom_codec_get_global_headers(&_ctx);

	// Allocate frames.
	_images.resize(_cfg.g_threads);
	for (auto& image : _images) {
		_factory->libaom_img_alloc(&image, _settings.color_format, _settings.width, _settings.height, 8);

		// Color Information.
		image.fmt        = _settings.color_format;
		image.cp         = _settings.color_primaries;
		image.tc         = _settings.color_trc;
		image.mc         = _settings.color_matrix;
		image.range      = _settings.color_range;
		image.monochrome = _settings.monochrome ? 1 : 0;
		image.csp        = AOM_CSP_VERTICAL; // !TODO: Consider making this user-controlled.

		// Size
		image.r_w = image.d_w;
		image.r_h = image.d_h;
		image.r_w = image.w;
		image.r_h = image.h;
	}

	// Log Settings
	log();

	// Signal to future update() calls that we are fully initialized.
	_initialized = true;
}

aom_av1_instance::~aom_av1_instance()
{
#ifdef ENABLE_PROFILING
	// Profiling
	D_LOG_INFO("Timings | Avg. µs       | 99.9ile µs    | 99.0ile µs    | 95.0ile µs    | Samples  ", "");
	D_LOG_INFO("--------+---------------+---------------+---------------+---------------+----------", "");
	D_LOG_INFO("Copy    | %13.1f | %13" PRId64 " | %13" PRId64 " | %13" PRId64 " | %9" PRIu64,
			   _profiler_copy->average_duration() / 1000.,
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_copy->percentile(0.999)).count(),
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_copy->percentile(0.990)).count(),
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_copy->percentile(0.950)).count(),
			   _profiler_copy->count());
	D_LOG_INFO("Encode  | %13.1f | %13" PRId64 " | %13" PRId64 " | %13" PRId64 " | %9" PRIu64,
			   _profiler_encode->average_duration() / 1000.,
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_encode->percentile(0.999)).count(),
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_encode->percentile(0.990)).count(),
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_encode->percentile(0.950)).count(),
			   _profiler_encode->count());
	D_LOG_INFO("Packet  | %13.1f | %13" PRId64 " | %13" PRId64 " | %13" PRId64 " | %9" PRIu64,
			   _profiler_packet->average_duration() / 1000.,
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_packet->percentile(0.999)).count(),
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_packet->percentile(0.990)).count(),
			   std::chrono::duration_cast<std::chrono::microseconds>(_profiler_packet->percentile(0.950)).count(),
			   _profiler_packet->count());
#endif

	// Deallocate global buffer.
	if (_global_headers) {
		/* Breaks heap
		if (_global_headers->buf) {
			free(_global_headers->buf);
			_global_headers->buf = nullptr;
		}
		free(_global_headers);
		_global_headers = nullptr;
		*/
	}

	// Deallocate frames.
	for (auto& image : _images) {
		_factory->libaom_img_free(&image);
	}
	_images.clear();

	// Destroy encoder.
	_factory->libaom_codec_destroy(&_ctx);
}

void aom_av1_instance::migrate(obs_data_t* settings, uint64_t version) {}

bool aom_av1_instance::update(obs_data_t* settings)
{
	video_t*                        obsVideo      = obs_encoder_video(_self);
	const struct video_output_info* obsVideoInfo  = video_output_get_info(obsVideo);
	uint32_t                        obsFPSnum     = obsVideoInfo->fps_num;
	uint32_t                        obsFPSden     = obsVideoInfo->fps_den;
	bool                            obsMonochrome = (obsVideoInfo->format == VIDEO_FORMAT_Y800);

#define SET_IF_NOT_DEFAULT(X, Y)         \
	if (X != -1) {                       \
		Y = static_cast<decltype(Y)>(X); \
	}

	{ // Generate Dynamic Settings

		{ // Encoder
			_settings.preset = static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_ENCODER_CPUUSAGE));
		}

		{ // Rate Control
			_settings.rc_bitrate = static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE));
			_settings.rc_bitrate_overshoot =
				static_cast<int32_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT));
			_settings.rc_bitrate_undershoot =
				static_cast<int32_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT));
			_settings.rc_quality = static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_QUALITY));
			_settings.rc_quantizer_min =
				static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM));
			_settings.rc_quantizer_max =
				static_cast<int8_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM));
			_settings.rc_buffer_ms = static_cast<int32_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE));
			_settings.rc_buffer_initial_ms =
				static_cast<int32_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL));
			_settings.rc_buffer_optimal_ms =
				static_cast<int32_t>(obs_data_get_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL));
		}

		{ // Key-Frames
			int64_t kf_type    = obs_data_get_int(settings, ST_KEY_KEYFRAMES_INTERVALTYPE);
			bool    is_seconds = (kf_type == 0);

			_settings.kf_mode = AOM_KF_AUTO;
			if (is_seconds) {
				_settings.kf_distance_max = static_cast<int32_t>(
					std::lround(obs_data_get_double(settings, ST_KEY_KEYFRAMES_INTERVAL_SECONDS)
								* static_cast<double>(obsFPSnum) / static_cast<double>(obsFPSden)));
			} else {
				_settings.kf_distance_max =
					static_cast<int32_t>(obs_data_get_int(settings, ST_KEY_KEYFRAMES_INTERVAL_FRAMES));
			}
			_settings.kf_distance_min = _settings.kf_distance_max;
		}

		// All-Intra requires us to never set Key-Frames.
		if (_cfg.g_usage == AOM_USAGE_ALL_INTRA) {
			_settings.rc_lookahead    = 0;
			_settings.kf_mode         = AOM_KF_DISABLED;
			_settings.kf_distance_min = 0;
			_settings.kf_distance_max = 0;
		}
	}

	{ // Configuration.

		{ // Usage and Defaults
			_cfg.g_usage = static_cast<unsigned int>(obs_data_get_int(settings, ST_KEY_ENCODER_USAGE));
			_factory->libaom_codec_enc_config_default(_iface, &_cfg, _cfg.g_usage);
		}

		{ // Frame Information
			// Size
			_cfg.g_w = _settings.width;
			_cfg.g_h = _settings.height;

			// Time Base (Rate is inverted Time Base)
			_cfg.g_timebase.num = static_cast<int>(_settings.fps.den);
			_cfg.g_timebase.den = static_cast<int>(_settings.fps.num);

			// !INFO: Whenever OBS decides to support anything but 8-bits, let me know.
			_cfg.g_bit_depth       = AOM_BITS_8;
			_cfg.g_input_bit_depth = AOM_BITS_8;

			// Monochrome color
			_cfg.monochrome = _settings.monochrome ? 1u : 0u;
		}

		{ // Encoder

			// AV1 Profile
			_cfg.g_profile = static_cast<unsigned int>(_settings.profile);
		}

		{ // Rate Control
			// Mode
			_cfg.rc_end_usage = static_cast<aom_rc_mode>(obs_data_get_int(settings, ST_KEY_RATECONTROL_MODE));

			// Look-Ahead
			SET_IF_NOT_DEFAULT(_settings.rc_lookahead, _cfg.g_lag_in_frames);

			// Limits
			SET_IF_NOT_DEFAULT(_settings.rc_bitrate, _cfg.rc_target_bitrate);
			SET_IF_NOT_DEFAULT(_settings.rc_bitrate_overshoot, _cfg.rc_overshoot_pct);
			SET_IF_NOT_DEFAULT(_settings.rc_bitrate_undershoot, _cfg.rc_undershoot_pct);
			SET_IF_NOT_DEFAULT(_settings.rc_quantizer_min, _cfg.rc_min_quantizer);
			SET_IF_NOT_DEFAULT(_settings.rc_quantizer_max, _cfg.rc_max_quantizer);

			// Buffer
			SET_IF_NOT_DEFAULT(_settings.rc_buffer_ms, _cfg.rc_buf_sz);
			SET_IF_NOT_DEFAULT(_settings.rc_buffer_initial_ms, _cfg.rc_buf_initial_sz);
			SET_IF_NOT_DEFAULT(_settings.rc_buffer_optimal_ms, _cfg.rc_buf_optimal_sz);
		}

		{ // Key-Frames
			SET_IF_NOT_DEFAULT(_settings.kf_mode, _cfg.kf_mode);
			SET_IF_NOT_DEFAULT(_settings.kf_distance_min, _cfg.kf_min_dist);
			SET_IF_NOT_DEFAULT(_settings.kf_distance_max, _cfg.kf_max_dist);
		}

		{ // Advanced

			// Single-Pass
			_cfg.g_pass = AOM_RC_ONE_PASS;

			// Threading
			SET_IF_NOT_DEFAULT(_settings.threads, _cfg.g_threads);
		}

		// TODO: Future
		//_cfg.rc_resize_mode = 0; // "RESIZE_NONE
		//_cfg.rc_resize_denominator = ?;
		//_cfg.rc_resize_kf_denominator = ?;
		//_cfg.rc_superres_mode = AOM_SUPERRES_NONE;
		//_cfg.rc_superres_denominator = ?;
		//_cfg.rc_superres_kf_denominator = ?;
		//_cfg.rc_superres_qthresh = ?;
		//_cfg.rc_superres_kf_qtresh = ?;
		//_cfg.rc_dropframe_thresh = ?;
		//_cfg.fwd_kf_enabled = ?;
		//_cfg.g_forced_max_frame_width = ?;
		//_cfg.g_forced_max_frame_height = ?;
		//_cfg.g_error_resilient = ?;
		//_cfg.g_lag_in_frames = ?;
		//_cfg.sframe_dist = ?;
		//_cfg.sframe_mode      = 0;
		//_cfg.large_scale_tile = 0;
		//_cfg.full_still_picture_hdr = ?;
		//_cfg.save_as_annexb = ?;
		//_cfg.encoder_cfg = ?;

		// Apply configuration
		if (_initialized) {
			if (auto error = _factory->libaom_codec_enc_config_set(&_ctx, &_cfg); error != AOM_CODEC_OK) {
				const char* errstr = _factory->libaom_codec_err_to_string(error);
				const char* err    = _factory->libaom_codec_error(&_ctx);
				const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
				D_LOG_WARNING("Error changing configuration: %s (code %" PRIu32 ")%s%s%s%s", //
							  (errstr ? errstr : ""), error,                                 //
							  (err ? "\n\tMessage: " : ""), (err ? err : ""),                //
							  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "")        //
				);
				return false;
			}
		}
	}

	if (_ctx.iface) { // Control

		{ // Encoder
#ifdef AOM_CTRL_AOME_SET_CPUUSED
			if (_settings.preset != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AOME_SET_CPUUSED, _settings.preset);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AOME_SET_CPUUSED",                                     //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif
		}

		{ // Rate Control
#ifdef AOM_CTRL_AOME_SET_CQ_LEVEL
			if ((_settings.rc_quality != -1) && ((_settings.rc_mode == AOM_CQ) || (_settings.rc_mode == AOM_Q))) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AOME_SET_CQ_LEVEL, _settings.rc_quality);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AOME_SET_CQ_LEVEL",                                    //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif
		}

		{ // Advanced
#ifdef AOM_CTRL_AV1E_SET_ROW_MT
			if (_settings.rowmultithreading != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_ROW_MT, _settings.rowmultithreading);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AV1E_SET_ROW_MT",                                      //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_COLUMNS
			if (_settings.tile_columns != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_TILE_COLUMNS, _settings.tile_columns);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AV1E_SET_TILE_COLUMNS",                                //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_ROWS
			if (_settings.tile_rows != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_TILE_ROWS, _settings.tile_rows);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AV1E_SET_TILE_ROWS",                                   //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif
#ifdef AOM_CTRL_AOME_SET_TUNING
			if (_settings.tune_metric != -1) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AOME_SET_TUNING, _settings.tune_metric);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AOME_SET_TUNING",                                      //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif
#ifdef AOM_CTRL_AV1E_SET_TUNE_CONTENT
			if (_settings.tune_content != AOM_CONTENT_DEFAULT) {
				if (auto error = _factory->libaom_codec_control(&_ctx, AV1E_SET_TUNE_CONTENT, _settings.tune_content);
					error != AOM_CODEC_OK) {
					const char* errstr = _factory->libaom_codec_err_to_string(error);
					const char* err    = _factory->libaom_codec_error(&_ctx);
					const char* errdtl = _factory->libaom_codec_error_detail(&_ctx);
					D_LOG_WARNING("Error changing '%s': %s (code %" PRIu32 ")%s%s%s%s",   //
								  "AV1E_SET_TUNE_CONTENT",                                //
								  (errstr ? errstr : ""), error,                          //
								  (err ? "\n\tMessage: " : ""), (err ? err : ""),         //
								  (errdtl ? "\n\tDetails: " : ""), (errdtl ? errdtl : "") //
					);
				}
			}
#endif
		}
	}

#undef SET_IF_NOT_DEFAULT

	// Log the changed settings.
	if (_initialized) {
		log();
	}

	return true;
}

void aom_av1_instance::log()
{
	D_LOG_INFO("AOM AV1:", "");
	D_LOG_INFO("  Video: %" PRIu16 "x%" PRIu16 "@%1.2ffps (%" PRIu32 "/%" PRIu32 ")", _settings.width, _settings.height,
			   static_cast<double>(_settings.fps.num) / static_cast<float>(_settings.fps.den), _settings.fps.num,
			   _settings.fps.den);
	D_LOG_INFO("  Color: %s/%s/%s%s", aom_color_format_to_string(_settings.color_format),
			   aom_color_trc_to_string(_settings.color_trc),
			   _settings.color_range == AOM_CR_FULL_RANGE ? "Full" : "Partial",
			   _settings.monochrome ? "/Monochrome" : "");

	// Rate Control
	D_LOG_INFO("  Rate Control: %s", aom_rc_mode_to_string(_settings.rc_mode));
	D_LOG_INFO("    Look-Ahead: %" PRId8, _settings.rc_lookahead);
	D_LOG_INFO("    Buffers: %" PRId32 " ms / %" PRId32 " ms / %" PRId32 " ms", _settings.rc_buffer_ms,
			   _settings.rc_buffer_initial_ms, _settings.rc_buffer_optimal_ms);
	D_LOG_INFO("    Bitrate: %" PRId32 " kbit/s (-%" PRId32 "%% - +%" PRId32 "%%)", _settings.rc_bitrate,
			   _settings.rc_bitrate_undershoot, _settings.rc_bitrate_overshoot);
	D_LOG_INFO("    Quality: %" PRId8, _settings.rc_quality);
	D_LOG_INFO("    Quantizer: %" PRId8 " - %" PRId8, _settings.rc_quantizer_min, _settings.rc_quantizer_max);

	// Key-Frames
	D_LOG_INFO("  Key-Frames: %s", aom_kf_mode_to_string(_settings.kf_mode));
	D_LOG_INFO("    Distance: %" PRId32 " - %" PRId32 " frames", _settings.kf_distance_min, _settings.kf_distance_max);

	// Advanced
	D_LOG_INFO("  Advanced: ", "");
	D_LOG_INFO("   Threads: %" PRId8, _settings.threads);
	D_LOG_INFO("   Row-Multi-Threading: %s", _settings.rowmultithreading == -1  ? "Default"
											 : _settings.rowmultithreading == 1 ? "Enabled"
																				: "Disabled");
	D_LOG_INFO("   Tiling: %" PRId8 "x%" PRId8, _settings.tile_columns, _settings.tile_rows);
	D_LOG_INFO("   Tune: %s (Metric), %s (Content)", aom_tune_metric_to_string(_settings.tune_metric),
			   aom_tune_content_to_string(_settings.tune_content));
}

bool aom_av1_instance::get_extra_data(uint8_t** extra_data, size_t* size)
{
	if (!_global_headers) {
		return false;
	}

	*extra_data = static_cast<uint8_t*>(_global_headers->buf);
	*size       = _global_headers->sz;

	return true;
}

bool aom_av1_instance::get_sei_data(uint8_t** sei_data, size_t* size)
{
	return get_extra_data(sei_data, size);
}

void aom_av1_instance::get_video_info(struct video_scale_info* info)
{
	// Fix up color format.
	auto format = obs_encoder_get_preferred_video_format(_self);
	if (format == VIDEO_FORMAT_NONE) {
		format = info->format;
	}

	switch (format) {
		// Perfect matches.
	case VIDEO_FORMAT_I444: // AOM_IMG_I444.
	case VIDEO_FORMAT_I422: // AOM_IMG_I422.
	case VIDEO_FORMAT_I420: // AOM_IMG_I420.
		break;

		// 4:2:0 formats
	case VIDEO_FORMAT_NV12: // Y, UV Interleaved.
	case VIDEO_FORMAT_I40A:
		D_LOG_WARNING("Color-format '%s' is not supported, forcing 'I420'...", obs_video_format_to_string(format));
		info->format = VIDEO_FORMAT_I420;
		break;

		// 4:2:2-like formats
	case VIDEO_FORMAT_UYVY:
	case VIDEO_FORMAT_YUY2:
	case VIDEO_FORMAT_YVYU:
	case VIDEO_FORMAT_I42A:
		D_LOG_WARNING("Color-format '%s' is not supported, forcing 'I422'...", obs_video_format_to_string(format));
		info->format = VIDEO_FORMAT_I422;
		break;

		// 4:4:4
	case VIDEO_FORMAT_BGR3:
	case VIDEO_FORMAT_BGRA:
	case VIDEO_FORMAT_BGRX:
	case VIDEO_FORMAT_RGBA:
	case VIDEO_FORMAT_YUVA:
	case VIDEO_FORMAT_AYUV:
	case VIDEO_FORMAT_Y800: // Grayscale, no exact match.
		D_LOG_WARNING("Color-format '%s' is not supported, forcing 'I444'...", obs_video_format_to_string(format));
		info->format = VIDEO_FORMAT_I444;
		break;
	default:
		throw std::runtime_error("Color Format is unknown.");
	}

	// Fix up color space.
	if (info->colorspace == VIDEO_CS_DEFAULT) {
		info->colorspace = VIDEO_CS_SRGB;
	}

	// Fix up color range.
	if (info->range == VIDEO_RANGE_DEFAULT) {
		info->range = VIDEO_RANGE_PARTIAL;
	}
}

bool streamfx::encoder::aom::av1::aom_av1_instance::encode_video(encoder_frame* frame, encoder_packet* packet,
																 bool* received_packet)
{
	// Retrieve current indexed image.
	auto& image = _images.at(_image_index);

	{ // Copy Image data.
#ifdef ENABLE_PROFILING
		auto profile = _profiler_copy->track();
#endif
		std::memcpy(image.planes[AOM_PLANE_Y], frame->data[0], frame->linesize[0] * image.h);
		if (image.fmt == AOM_IMG_FMT_I420) {
			std::memcpy(image.planes[AOM_PLANE_U], frame->data[1], frame->linesize[1] * image.h / 2);
			std::memcpy(image.planes[AOM_PLANE_V], frame->data[2], frame->linesize[2] * image.h / 2);
		} else {
			std::memcpy(image.planes[AOM_PLANE_U], frame->data[1], frame->linesize[1] * image.h);
			std::memcpy(image.planes[AOM_PLANE_V], frame->data[2], frame->linesize[2] * image.h);
		}
	}

	{ // Try to encode the new image.
#ifdef ENABLE_PROFILING
		auto profile = _profiler_encode->track();
#endif
		aom_enc_frame_flags_t flags = 0;
		if (_cfg.g_usage == AOM_USAGE_ALL_INTRA) {
			flags = AOM_EFLAG_FORCE_KF;
		}
		if (auto error = _factory->libaom_codec_encode(&_ctx, &image, frame->pts, 1, flags); error != AOM_CODEC_OK) {
			const char* errstr = _factory->libaom_codec_err_to_string(error);
			D_LOG_ERROR("Encoding frame failed with error: %s (code %" PRIu32 ")\n%s\n%s", errstr, error,
						_factory->libaom_codec_error(&_ctx), _factory->libaom_codec_error_detail(&_ctx));
			return false;
		} else {
			// Increment the image index.
			_image_index = (_image_index++) % _images.size();
		}
	}

	{ // Get Packet
#ifdef ENABLE_PROFILING
		auto profile = _profiler_packet->track();
#endif
		aom_codec_iter_t iter = NULL;
		for (auto* pkt = _factory->libaom_codec_get_cx_data(&_ctx, &iter); pkt != nullptr;
			 pkt       = _factory->libaom_codec_get_cx_data(&_ctx, &iter)) {
#ifdef _DEBUG
			{
				const char* kind = "";
				switch (pkt->kind) {
				case AOM_CODEC_CX_FRAME_PKT:
					kind = "Frame";
					break;
				case AOM_CODEC_STATS_PKT:
					kind = "Stats";
					break;
				case AOM_CODEC_FPMB_STATS_PKT:
					kind = "FPMB Stats";
					break;
				case AOM_CODEC_PSNR_PKT:
					kind = "PSNR";
					break;
				case AOM_CODEC_CUSTOM_PKT:
					kind = "Custom";
					break;
				}
				D_LOG_DEBUG("\tPacket: Kind=%s", kind)
			}
#endif

			if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
				// Status
				packet->type     = OBS_ENCODER_VIDEO;
				packet->keyframe = ((pkt->data.frame.flags & AOM_FRAME_IS_KEY) == AOM_FRAME_IS_KEY)
								   || (_cfg.g_usage == AOM_USAGE_ALL_INTRA);
				if (packet->keyframe) {
					//
					packet->priority      = 3; // OBS_NAL_PRIORITY_HIGHEST
					packet->drop_priority = 3; // OBS_NAL_PRIORITY_HIGHEST
				} else if ((pkt->data.frame.flags & AOM_FRAME_IS_DROPPABLE) != AOM_FRAME_IS_DROPPABLE) {
					// Dropping this frame breaks the bitstream.
					packet->priority      = 2; // OBS_NAL_PRIORITY_HIGH
					packet->drop_priority = 3; // OBS_NAL_PRIORITY_HIGHEST
				} else {
					// This frame can be dropped at will.
					packet->priority      = 0; // OBS_NAL_PRIORITY_DISPOSABLE
					packet->drop_priority = 0; // OBS_NAL_PRIORITY_DISPOSABLE
				}

				// Data
				packet->data = static_cast<uint8_t*>(pkt->data.frame.buf);
				packet->size = pkt->data.frame.sz;

				// Timestamps
				//TODO: Temporarily set both to the same until there is a way to figure out actual order.
				packet->pts = pkt->data.frame.pts;
				packet->dts = pkt->data.frame.pts;

				*received_packet = true;
			}

			if (*received_packet == true)
				break;
		}

		if (!*received_packet) {
			packet->type = OBS_ENCODER_VIDEO;
			packet->data = nullptr;
			packet->size = 0;
			packet->pts  = -1;
			packet->dts  = -1;
#ifdef _DEBUG
			D_LOG_DEBUG("No Packet", "");
#endif
			// Not necessarily an error.
			//return false;
		} else {
#ifdef _DEBUG
			D_LOG_DEBUG("Packet: Type=%s PTS=%06" PRId64 " DTS=%06" PRId64 " Size=%016" PRIuPTR "",
						packet->keyframe ? "I" : "P", packet->pts, packet->dts, packet->size);
#endif
		}
	}

	return true;
}

aom_av1_factory::aom_av1_factory()
{
	// Try and load the AOM library.
	std::vector<std::filesystem::path> libs;

#ifdef D_PLATFORM_WINDOWS
	// Try loading from the data directory first.
	libs.push_back(streamfx::data_file_path("aom.dll"));    // MSVC (preferred)
	libs.push_back(streamfx::data_file_path("libaom.dll")); // Cross-Compile
	// In any other case, load the system-wide binary.
	libs.push_back("aom.dll");
	libs.push_back("libaom.dll");
#else
	// Try loading from the data directory first.
	libs.push_back(streamfx::data_file_path("libaom.so"));
	// In any other case, load the system-wide binary.
	libs.push_back("libaom");
#endif

	for (auto lib : libs) {
		try {
			_library = streamfx::util::library::load(lib);
			if (_library)
				break;
		} catch (...) {
			D_LOG_WARNING("Loading of '%s' failed.", lib.generic_string().c_str());
		}
	}
	if (!_library) {
		throw std::runtime_error("Unable to load AOM library.");
	}

	// Load all necessary functions.
#define _LOAD_SYMBOL(X)                                                                      \
	{                                                                                        \
		lib##X = reinterpret_cast<decltype(lib##X)>(_library->load_symbol(std::string(#X))); \
	}
#define _LOAD_SYMBOL_(X, Y)                                                                 \
	{                                                                                       \
		lib##X = reinterpret_cast<decltype(lib##X)>(_library->load_symbol(std::string(Y))); \
	}
	_LOAD_SYMBOL(aom_codec_version);
	_LOAD_SYMBOL(aom_codec_version_str);
	_LOAD_SYMBOL(aom_codec_version_extra_str);
	_LOAD_SYMBOL(aom_codec_build_config);
	_LOAD_SYMBOL(aom_codec_iface_name);
	_LOAD_SYMBOL(aom_codec_err_to_string);
	_LOAD_SYMBOL(aom_codec_error);
	_LOAD_SYMBOL(aom_codec_error_detail);
	_LOAD_SYMBOL(aom_codec_destroy);
	_LOAD_SYMBOL(aom_codec_get_caps);
	_LOAD_SYMBOL(aom_codec_control);
	_LOAD_SYMBOL(aom_codec_set_option);
	_LOAD_SYMBOL(aom_obu_type_to_string);
	_LOAD_SYMBOL(aom_uleb_size_in_bytes);
	_LOAD_SYMBOL(aom_uleb_decode);
	_LOAD_SYMBOL(aom_uleb_encode);
	_LOAD_SYMBOL(aom_uleb_encode_fixed_size);
	_LOAD_SYMBOL(aom_img_alloc);
	_LOAD_SYMBOL(aom_img_wrap);
	_LOAD_SYMBOL(aom_img_alloc_with_border);
	_LOAD_SYMBOL(aom_img_set_rect);
	_LOAD_SYMBOL(aom_img_flip);
	_LOAD_SYMBOL(aom_img_free);
	_LOAD_SYMBOL(aom_img_plane_width);
	_LOAD_SYMBOL(aom_img_plane_height);
	_LOAD_SYMBOL(aom_img_add_metadata);
	_LOAD_SYMBOL(aom_img_get_metadata);
	_LOAD_SYMBOL(aom_img_num_metadata);
	_LOAD_SYMBOL(aom_img_remove_metadata);
	_LOAD_SYMBOL(aom_img_metadata_alloc);
	_LOAD_SYMBOL(aom_img_metadata_free);
	_LOAD_SYMBOL(aom_codec_enc_init_ver);
	_LOAD_SYMBOL(aom_codec_enc_config_default);
	_LOAD_SYMBOL(aom_codec_enc_config_set);
	_LOAD_SYMBOL(aom_codec_get_global_headers);
	_LOAD_SYMBOL(aom_codec_encode);
	_LOAD_SYMBOL(aom_codec_set_cx_data_buf);
	_LOAD_SYMBOL(aom_codec_get_cx_data);
	_LOAD_SYMBOL(aom_codec_get_preview_frame);
	_LOAD_SYMBOL(aom_codec_av1_cx);
#undef _LOAD_SYMBOL

	// Register encoder.
	_info.id    = S_PREFIX "aom-av1";
	_info.type  = obs_encoder_type::OBS_ENCODER_VIDEO;
	_info.codec = "av1";
	_info.caps  = OBS_ENCODER_CAP_DYN_BITRATE;
	_info.caps |= OBS_ENCODER_CAP_DEPRECATED;

	finish_setup();
}

aom_av1_factory::~aom_av1_factory() {}

std::shared_ptr<aom_av1_factory> _aom_av1_factory_instance = nullptr;

void aom_av1_factory::initialize()
{
	try {
		if (!_aom_av1_factory_instance) {
			_aom_av1_factory_instance = std::make_shared<aom_av1_factory>();
		}
	} catch (std::exception const& ex) {
		D_LOG_ERROR("Failed to initialize AOM AV1 encoder: %s", ex.what());
	}
}

void aom_av1_factory::finalize()
{
	_aom_av1_factory_instance.reset();
}

std::shared_ptr<aom_av1_factory> aom_av1_factory::get()
{
	return _aom_av1_factory_instance;
}

const char* aom_av1_factory::get_name()
{
	return "AV1 (via AOM)";
}

void* aom_av1_factory::create(obs_data_t* settings, obs_encoder_t* encoder, bool is_hw)
{
	return new aom_av1_instance(settings, encoder, is_hw);
}

void aom_av1_factory::get_defaults2(obs_data_t* settings)
{
	{ // Presets
		obs_data_set_default_int(settings, ST_KEY_ENCODER_USAGE, static_cast<long long>(AOM_USAGE_REALTIME));
		obs_data_set_default_int(settings, ST_KEY_ENCODER_CPUUSAGE, -1);
		obs_data_set_default_int(settings, ST_KEY_ENCODER_PROFILE,
								 static_cast<long long>(codec::av1::profile::UNKNOWN));
	}

	{ // Rate-Control
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_MODE, static_cast<long long>(AOM_CBR));

		// Limits
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE, 6000);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_QUALITY, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM, -1);

		// Buffer
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL, -1);
		obs_data_set_default_int(settings, ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL, -1);
	}

	{ // Key-Frame Options
		obs_data_set_default_int(settings, ST_KEY_KEYFRAMES_INTERVALTYPE, 0);
		obs_data_set_default_double(settings, ST_KEY_KEYFRAMES_INTERVAL_SECONDS, 2.0);
		obs_data_set_default_int(settings, ST_KEY_KEYFRAMES_INTERVAL_FRAMES, 300);
	}

	{ // Advanced Options
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_THREADS, 0);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_ROWMULTITHREADING, -1);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_TILE_COLUMNS, -1);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_TILE_ROWS, -1);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_TUNE_METRIC, -1);
		obs_data_set_default_int(settings, ST_KEY_ADVANCED_TUNE_CONTENT, static_cast<long long>(AOM_CONTENT_DEFAULT));
	}
}

static bool modified_usage(obs_properties_t* props, obs_property_t*, obs_data_t* settings) noexcept
{
	try {
		bool is_all_intra = false;
		if (obs_data_get_int(settings, ST_KEY_ENCODER_USAGE) == AOM_USAGE_ALL_INTRA) {
			is_all_intra = true;
		}

		// All-Intra does not support these.
		obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LOOKAHEAD), !is_all_intra);
		obs_property_set_visible(obs_properties_get(props, ST_I18N_KEYFRAMES), !is_all_intra);

		return true;
	} catch (const std::exception& ex) {
		DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		return false;
	} catch (...) {
		DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		return false;
	}
}

static bool modified_ratecontrol_mode(obs_properties_t* props, obs_property_t*, obs_data_t* settings) noexcept
{
	try {
		bool is_bitrate_visible        = false;
		bool is_overundershoot_visible = false;
		bool is_quality_visible        = false;

		// Fix rate control mode selection if ALL_INTRA is selected.
		if (obs_data_get_int(settings, ST_KEY_ENCODER_USAGE) == AOM_USAGE_ALL_INTRA) {
			obs_data_set_int(settings, ST_KEY_RATECONTROL_MODE, static_cast<long long>(aom_rc_mode::AOM_Q));
		}

		{ // Based on the Rate Control Mode, show and hide options.
			auto mode = static_cast<aom_rc_mode>(obs_data_get_int(settings, ST_KEY_RATECONTROL_MODE));
			if (mode == AOM_CBR) {
				is_bitrate_visible        = true;
				is_overundershoot_visible = true;
			} else if (mode == AOM_VBR) {
				is_bitrate_visible        = true;
				is_overundershoot_visible = true;
			} else if (mode == AOM_CQ) {
				is_bitrate_visible        = true;
				is_overundershoot_visible = true;
				is_quality_visible        = true;
			} else if (mode == AOM_Q) {
				is_quality_visible = true;
			}

			obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_BITRATE), is_bitrate_visible);
			obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT),
									 is_overundershoot_visible);
			obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT),
									 is_overundershoot_visible);
#ifdef AOM_CTRL_AOME_SET_CQ_LEVEL
			obs_property_set_visible(obs_properties_get(props, ST_KEY_RATECONTROL_LIMITS_QUALITY), is_quality_visible);
#endif
		}
		return true;
	} catch (const std::exception& ex) {
		DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		return false;
	} catch (...) {
		DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		return false;
	}
}

static bool modified_keyframes(obs_properties_t* props, obs_property_t*, obs_data_t* settings) noexcept
{
	try {
		bool is_seconds = obs_data_get_int(settings, ST_KEY_KEYFRAMES_INTERVALTYPE) == 0;
		obs_property_set_visible(obs_properties_get(props, ST_KEY_KEYFRAMES_INTERVAL_FRAMES), !is_seconds);
		obs_property_set_visible(obs_properties_get(props, ST_KEY_KEYFRAMES_INTERVAL_SECONDS), is_seconds);
		return true;
	} catch (const std::exception& ex) {
		DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		return false;
	} catch (...) {
		DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		return false;
	}
}

obs_properties_t* aom_av1_factory::get_properties2(instance_t* data)
{
	obs_properties_t* props = obs_properties_create();

	{
		auto p = obs_properties_add_text(props, "[[deprecated]]", D_TRANSLATE(ST_I18N_DEPRECATED), OBS_TEXT_INFO);
		obs_property_text_set_info_type(p, OBS_TEXT_INFO_WARNING);
		obs_property_text_set_info_word_wrap(p, true);
	}

#ifdef ENABLE_FRONTEND
	{
		obs_properties_add_button2(props, S_MANUAL_OPEN, D_TRANSLATE(S_MANUAL_OPEN), aom_av1_factory::on_manual_open,
								   this);
	}
#endif

	{ // Presets
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_ENCODER, D_TRANSLATE(ST_I18N_ENCODER), OBS_GROUP_NORMAL, grp);
		//obs_properties_add_group(props, S_CODEC_AV1, D_TRANSLATE(S_CODEC_AV1), OBS_GROUP_NORMAL, grp);

		{ // Usage
			auto p = obs_properties_add_list(grp, ST_KEY_ENCODER_USAGE, D_TRANSLATE(ST_I18N_ENCODER_USAGE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_modified_callback(p, modified_usage);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_USAGE_GOODQUALITY),
									  static_cast<long long>(AOM_USAGE_GOOD_QUALITY));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_USAGE_REALTIME),
									  static_cast<long long>(AOM_USAGE_REALTIME));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_USAGE_ALLINTRA),
									  static_cast<long long>(AOM_USAGE_ALL_INTRA));
		}

#ifdef AOM_CTRL_AOME_SET_CPUUSED
		{ // CPU Usage
			auto p = obs_properties_add_list(grp, ST_KEY_ENCODER_CPUUSAGE, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), -1);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_10), 10);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_9), 9);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_8), 8);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_7), 7);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_6), 6);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_5), 5);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_4), 4);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_3), 3);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_2), 2);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_1), 1);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ENCODER_CPUUSAGE_0), 0);
		}
#endif

		{ // Profile
			auto p = obs_properties_add_list(grp, ST_KEY_ENCODER_PROFILE, D_TRANSLATE(S_CODEC_AV1_PROFILE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_AUTOMATIC),
									  static_cast<long long>(codec::av1::profile::UNKNOWN));
			obs_property_list_add_int(p, codec::av1::profile_to_string(codec::av1::profile::MAIN),
									  static_cast<long long>(codec::av1::profile::MAIN));
			obs_property_list_add_int(p, codec::av1::profile_to_string(codec::av1::profile::HIGH),
									  static_cast<long long>(codec::av1::profile::HIGH));
			obs_property_list_add_int(p, codec::av1::profile_to_string(codec::av1::profile::PROFESSIONAL),
									  static_cast<long long>(codec::av1::profile::PROFESSIONAL));
		}
	}

	{ // Rate Control Options
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_RATECONTROL, D_TRANSLATE(ST_I18N_RATECONTROL), OBS_GROUP_NORMAL, grp);

		{ // Mode
			auto p = obs_properties_add_list(grp, ST_KEY_RATECONTROL_MODE, D_TRANSLATE(ST_I18N_RATECONTROL_MODE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_modified_callback(p, modified_ratecontrol_mode);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_VBR), static_cast<long long>(AOM_VBR));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_CBR), static_cast<long long>(AOM_CBR));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_CQ), static_cast<long long>(AOM_CQ));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_RATECONTROL_MODE_Q), static_cast<long long>(AOM_Q));
		}

		{ // Look-Ahead
			auto p =
				obs_properties_add_int(grp, ST_KEY_RATECONTROL_LOOKAHEAD, D_TRANSLATE(ST_I18N_RATECONTROL_LOOKAHEAD),
									   -1, std::numeric_limits<int32_t>::max(), 1);
			obs_property_int_set_suffix(p, " frames");
		}

		{ // Limits
			obs_properties_t* grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_I18N_RATECONTROL_LIMITS, D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS),
									 OBS_GROUP_NORMAL, grp2);

			{ // Bitrate
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_LIMITS_BITRATE,
												D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_BITRATE), 0,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " kbit/s");
			}

			{ // Bitrate Under/Overshoot
				auto p1 = obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT,
														D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_BITRATE_UNDERSHOOT), -1,
														100, 1);
				auto p2 = obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_BITRATE_OVERSHOOT,
														D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_BITRATE_OVERSHOOT), -1,
														100, 1);
				obs_property_float_set_suffix(p1, " %");
				obs_property_float_set_suffix(p2, " %");
			}

#ifdef AOM_CTRL_AOME_SET_CQ_LEVEL
			{ // Quality
				auto p = obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_QUALITY,
													   D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUALITY), -1, 63, 1);
			}
#endif

			{ // Quantizer
				auto p1 =
					obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MINIMUM,
												  D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MINIMUM), -1, 63, 1);
				auto p2 =
					obs_properties_add_int_slider(grp2, ST_KEY_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM,
												  D_TRANSLATE(ST_I18N_RATECONTROL_LIMITS_QUANTIZER_MAXIMUM), -1, 63, 1);
			}
		}

		{ // Buffer
			obs_properties_t* grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_I18N_RATECONTROL_BUFFER, D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER),
									 OBS_GROUP_NORMAL, grp2);

			{ // Buffer Size
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_BUFFER_SIZE,
												D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER_SIZE), -1,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " ms");
			}

			{ // Initial Buffer Size
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_BUFFER_SIZE_INITIAL,
												D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER_SIZE_INITIAL), -1,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " ms");
			}

			{ // Optimal Buffer Size
				auto p = obs_properties_add_int(grp2, ST_KEY_RATECONTROL_BUFFER_SIZE_OPTIMAL,
												D_TRANSLATE(ST_I18N_RATECONTROL_BUFFER_SIZE_OPTIMAL), -1,
												std::numeric_limits<int32_t>::max(), 1);
				obs_property_int_set_suffix(p, " ms");
			}
		}
	}

	{ // Key-Frame Options
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_KEYFRAMES, D_TRANSLATE(ST_I18N_KEYFRAMES), OBS_GROUP_NORMAL, grp);

		{ // Key-Frame Interval Type
			auto p =
				obs_properties_add_list(grp, ST_KEY_KEYFRAMES_INTERVALTYPE, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVALTYPE),
										OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_modified_callback(p, modified_keyframes);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVALTYPE_SECONDS), 0);
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVALTYPE_FRAMES), 1);
		}

		{ // Key-Frame Interval Seconds
			auto p = obs_properties_add_float(grp, ST_KEY_KEYFRAMES_INTERVAL_SECONDS,
											  D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVAL), 0.00,
											  std::numeric_limits<uint16_t>::max(), 0.01);
			obs_property_float_set_suffix(p, " seconds");
		}

		{ // Key-Frame Interval Frames
			auto p =
				obs_properties_add_int(grp, ST_KEY_KEYFRAMES_INTERVAL_FRAMES, D_TRANSLATE(ST_I18N_KEYFRAMES_INTERVAL),
									   0, std::numeric_limits<int32_t>::max(), 1);
			obs_property_int_set_suffix(p, " frames");
		}
	}

	{ // Advanced Options
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(props, ST_I18N_ADVANCED, D_TRANSLATE(ST_I18N_ADVANCED), OBS_GROUP_NORMAL, grp);

		{ // Threads
			auto p = obs_properties_add_int(grp, ST_KEY_ADVANCED_THREADS, D_TRANSLATE(ST_I18N_ADVANCED_THREADS), 0,
											std::numeric_limits<int32_t>::max(), 1);
		}

#ifdef AOM_CTRL_AV1E_SET_ROW_MT
		{ // Row-MT
			auto p = streamfx::util::obs_properties_add_tristate(grp, ST_KEY_ADVANCED_ROWMULTITHREADING,
																 D_TRANSLATE(ST_I18N_ADVANCED_ROWMULTITHREADING));
		}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_COLUMNS
		{ // Tile Columns
			auto p = obs_properties_add_int_slider(grp, ST_KEY_ADVANCED_TILE_COLUMNS,
												   D_TRANSLATE(ST_I18N_ADVANCED_TILE_COLUMNS), -1, 6, 1);
		}
#endif

#ifdef AOM_CTRL_AV1E_SET_TILE_ROWS
		{ // Tile Rows
			auto p = obs_properties_add_int_slider(grp, ST_KEY_ADVANCED_TILE_ROWS,
												   D_TRANSLATE(ST_I18N_ADVANCED_TILE_ROWS), -1, 6, 1);
		}
#endif
		{
			obs_properties_t* grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_I18N_ADVANCED_TUNE, D_TRANSLATE(ST_I18N_ADVANCED_TUNE), OBS_GROUP_NORMAL,
									 grp2);

#ifdef AOM_CTRL_AOME_SET_TUNING
			{ // Tuning
				auto p = obs_properties_add_list(grp2, ST_KEY_ADVANCED_TUNE_METRIC,
												 D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC), OBS_COMBO_TYPE_LIST,
												 OBS_COMBO_FORMAT_INT);
				obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), -1);
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_PSNR),
										  static_cast<long long>(AOM_TUNE_PSNR));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_SSIM),
										  static_cast<long long>(AOM_TUNE_SSIM));
#ifdef _DEBUG
				// These have no actual use outside of debug builds, way too slow!
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_VMAFWITHPREPROCESSING),
										  static_cast<long long>(AOM_TUNE_VMAF_WITH_PREPROCESSING));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_VMAFWITHOUTPREPROCESSING),
										  static_cast<long long>(AOM_TUNE_VMAF_WITHOUT_PREPROCESSING));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_VMAFMAXGAIN),
										  static_cast<long long>(AOM_TUNE_VMAF_MAX_GAIN));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_VMAFNEGMAXGAIN),
										  static_cast<long long>(AOM_TUNE_VMAF_NEG_MAX_GAIN));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_METRIC_BUTTERAUGLI),
										  static_cast<long long>(AOM_TUNE_BUTTERAUGLI));
#endif
			}
#endif

#ifdef AOM_CTRL_AV1E_SET_TUNE_CONTENT
			{ // Content Tuning
				auto p = obs_properties_add_list(grp2, ST_KEY_ADVANCED_TUNE_CONTENT,
												 D_TRANSLATE(ST_I18N_ADVANCED_TUNE_CONTENT), OBS_COMBO_TYPE_LIST,
												 OBS_COMBO_FORMAT_INT);
				obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), static_cast<long long>(AOM_CONTENT_DEFAULT));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_CONTENT_SCREEN),
										  static_cast<long long>(AOM_CONTENT_SCREEN));
				obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_TUNE_CONTENT_FILM),
										  static_cast<long long>(AOM_CONTENT_FILM));
			}
#endif
		}
	}

	return props;
}

#ifdef ENABLE_FRONTEND
bool aom_av1_factory::on_manual_open(obs_properties_t* props, obs_property_t* property, void* data)
{
	streamfx::open_url(HELP_URL);
	return false;
}
#endif
