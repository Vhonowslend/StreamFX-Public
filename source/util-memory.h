/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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
#include <cstdlib>

namespace util {
	inline size_t aligned_offset(size_t align, size_t pos)
	{
		return ((pos / align) + 1) * align;
	}
	void* malloc_aligned(size_t align, size_t size);
	void  free_aligned(void* mem);

	template<typename T, size_t N = 16>
	class AlignmentAllocator {
		public:
		typedef T              value_type;
		typedef size_t         size_type;
		typedef std::ptrdiff_t difference_type;

		typedef T*       pointer;
		typedef const T* const_pointer;

		typedef T&       reference;
		typedef const T& const_reference;

		public:
		inline AlignmentAllocator() throw() {}

		template<typename T2>
		inline AlignmentAllocator(const AlignmentAllocator<T2, N>&) throw()
		{}

		inline ~AlignmentAllocator() throw() {}

		inline pointer adress(reference r)
		{
			return &r;
		}

		inline const_pointer adress(const_reference r) const
		{
			return &r;
		}

		inline pointer allocate(size_type n)
		{
			return (pointer)malloc_aligned(n * sizeof(value_type), N);
		}

		inline void deallocate(pointer p, size_type)
		{
			free_aligned(p);
		}

		inline void construct(pointer p, const value_type& wert)
		{
			new (p) value_type(wert);
		}

		inline void destroy(pointer p)
		{
			p->~value_type();
			p;
		}

		inline size_type max_size() const throw()
		{
			return size_type(-1) / sizeof(value_type);
		}

		template<typename T2>
		struct rebind {
			typedef AlignmentAllocator<T2, N> other;
		};

		bool operator!=(const AlignmentAllocator<T, N>& other) const
		{
			return !(*this == other);
		}

		// Returns true if and only if storage allocated from *this
		// can be deallocated from other, and vice versa.
		// Always returns true for stateless allocators.
		bool operator==(const AlignmentAllocator<T, N>& other) const
		{
			return true;
		}
	};
}; // namespace util
