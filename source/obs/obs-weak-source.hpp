/*
 * Copyright (C) 2022 Michael Fabian Dirks
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

#pragma once
#include "common.hpp"
#include "obs-source.hpp"

namespace streamfx::obs {
	class source;

	class weak_source {
		obs_weak_source_t* _ref;

		public:
		~weak_source()
		{
			if (_ref) {
				obs_weak_source_release(_ref);
			}
		};

		/** Empty/Invalid weak reference.
		 *
		 */
		weak_source() : _ref(nullptr){};

		/** Create a new weak reference from an existing pointer.
		 *
		 * Attention: Ownership of obs_weak_source_t is transferred to the class itself, and should not be released.
		 */
		weak_source(obs_weak_source_t* source) : _ref(source)
		{
			if (!_ref)
				throw std::invalid_argument("Parameter 'source' does not define a valid source.");
		};

		/** Create a new weak reference from an existing hard reference.
		 */
		weak_source(obs_source_t* source)
		{
			_ref = obs_source_get_weak_source(source);
			if (!_ref)
				throw std::invalid_argument("Parameter 'source' does not define a valid source.");
		};

		/** Create a new weak reference from an existing hard reference.
		 */
		weak_source(::streamfx::obs::source& source)
		{
			_ref = obs_source_get_weak_source(source.get());
			if (!_ref)
				throw std::invalid_argument("Parameter 'source' does not define a valid source.");
		};

		/** Create a new weak reference for a given source by name.
		 *
		 * Attention: May fail if the name does not exactly match.
		 */
		weak_source(std::string_view name)
		{
			std::shared_ptr<obs_source_t> ref{obs_get_source_by_name(name.data()),
											  [](obs_source_t* v) { obs_source_release(v); }};
			if (!ref) {
				throw std::invalid_argument("Parameter 'name' does not define an valid source.");
			}
			_ref = obs_source_get_weak_source(ref.get());
		};

		weak_source(weak_source&& move) noexcept
		{
			_ref      = move._ref;
			move._ref = nullptr;
		};

		FORCE_INLINE ::streamfx::obs::weak_source& operator=(weak_source&& move) noexcept
		{
			if (_ref) {
				obs_weak_source_release(_ref);
				_ref = nullptr;
			}
			if (move._ref) {
				_ref      = move._ref;
				move._ref = nullptr;
			}
			return *this;
		};

		weak_source(const weak_source& copy)
		{
			_ref = copy._ref;
			obs_weak_source_addref(_ref);
		};

		FORCE_INLINE ::streamfx::obs::weak_source& operator=(const weak_source& copy)
		{
			if (_ref) {
				obs_weak_source_release(_ref);
				_ref = nullptr;
			}
			if (copy._ref) {
				_ref = copy._ref;
				obs_weak_source_addref(_ref);
			}

			return *this;
		};

		/** Retrieve the underlying pointer for manual manipulation.
		 *
		 * Attention: Ownership remains with the class instance.
		 */
		FORCE_INLINE obs_weak_source_t* get() const
		{
			return _ref;
		};

		/** Is the weak reference expired?
		 *
		 * A weak reference is expired when the original object it is pointing at no longer exists.
		 */
		FORCE_INLINE bool expired() const
		{
			return (!_ref) || (obs_weak_source_expired(_ref));
		};

		/** Try and acquire a hard reference to the source.
		 *
		 * May fail if the reference expired before we successfully acquire it.
		 */
		FORCE_INLINE ::streamfx::obs::source lock() const
		{
			return {obs_weak_source_get_source(_ref)};
		};

		public:
		FORCE_INLINE operator obs_weak_source_t*() const
		{
			return _ref;
		}

		FORCE_INLINE obs_weak_source_t* operator*() const
		{
			return _ref;
		}

		FORCE_INLINE operator bool() const
		{
			return !expired();
		};

		FORCE_INLINE bool operator==(weak_source const& rhs) const
		{
			return _ref == rhs._ref;
		};

		FORCE_INLINE bool operator<(weak_source const& rhs) const
		{
			return _ref < rhs._ref;
		};

		FORCE_INLINE bool operator==(obs_weak_source_t* const& rhs) const
		{
			return _ref == rhs;
		};

		FORCE_INLINE bool operator==(source const& rhs) const
		{
			return obs_weak_source_references_source(_ref, rhs.get());
		};

		FORCE_INLINE bool operator==(obs_source_t* const& rhs) const
		{
			return obs_weak_source_references_source(_ref, rhs);
		};
	};
} // namespace streamfx::obs
