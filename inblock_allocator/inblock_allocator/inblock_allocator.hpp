#ifndef INBLOCK_ALLOCATOR
#define INBLOCK_ALLOCATOR

#include <cstdint>
#include <iostream>
#include <typeinfo>

const size_t chunk_size = 128; // How many records each bitmap has
const size_t big_chunk = 64;    // Anything that is bigger than 64 Bytes is allocated as big chunk
using byte_ptr = std::uint8_t *;
class inblock_allocator_heap;
class Chunk;

class Chunk
{
public:
	Chunk * next;
	Chunk * previous;
	size_t chunk_type;
	bool bitmap;


	Chunk(Chunk * n, Chunk * p, size_t ch, bool b) : next(n), previous(p), chunk_type(ch), bitmap(b) {}

	static size_t size()
	{
		return sizeof(Chunk);
	}

	size_t total_size(size_t align_value, byte_ptr ptr)
	{
		if (bitmap)
		{                                                       // $this should work the same as $ptr
			uintptr_t align = (uintptr_t)((uintptr_t)(ptr)) + size() + chunk_size / 8;
			align = 8 - align % 8;
			return (size_t)(align + size() + chunk_size / 8 + chunk_size * chunk_type);
		}
		else
		{
			uintptr_t align = (uintptr_t)((uintptr_t)(ptr)) + size();
			align = align_value - align % align_value;
			return (size_t)(align + chunk_type + size());
		}
	}
};

class inblock_allocator_heap
{
public:
	byte_ptr heap_ptr;
	size_t heap_size;
	Chunk * first_chunk;

	void operator() (void * ptr, size_t n_bytes)
	{
		uintptr_t align = (uintptr_t)(ptr);
		align = 8 - align % 8;
		heap_ptr = (byte_ptr)ptr + align;
		first_chunk = nullptr;
		heap_size = n_bytes;
	}
};


template<typename T, typename HeapHolder>
class inblock_allocator
{
public:
	using allocator_type = inblock_allocator;
	using pointer = T * ;
	using const_pointer = const T*;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T & ;
	using const_reference = const T&;
	using value_type = T;
	using is_always_true = std::true_type;

	inblock_allocator<T, HeapHolder>() {}

	template<typename U> inblock_allocator(const inblock_allocator<U, HeapHolder> & other) noexcept {}
	template<typename U> inblock_allocator(inblock_allocator<U, HeapHolder> && other) noexcept {}
	//template<typename U> inblock_allocator(inblock_allocator<U, HeapHolder> const& other) noexcept {}

	pointer allocate(size_t n)
	{
		if (n <= chunk_size && sizeof(value_type) <= big_chunk)
		{                                       //bitmaps
			size_t size = determine_correct_chunk_size();
			Chunk * ptr = HeapHolder::heap.first_chunk;
			while (ptr != nullptr)
			{
				if (ptr->bitmap && ptr->chunk_type == size)
				{
					pointer res = find_free_space((byte_ptr)ptr + Chunk::size(), chunk_size / 8, size, n);
					if (res)
						return res;
				}
				ptr = ptr->next;
			}
			return insert_new_bmp_chunk(size, n);
		}
		else // big chunk
			return insert_new_big_chunk(sizeof(value_type), n);
	}

	void deallocate(pointer to_dealloc, size_t n)
	{
		Chunk * ptr = HeapHolder::heap.first_chunk;
		while (ptr != nullptr)
		{
			if ((uintptr_t)ptr < (uintptr_t)to_dealloc && (uintptr_t)to_dealloc < (uintptr_t)((uintptr_t)ptr + ptr->total_size(8, (byte_ptr)ptr)))
			{
				if (ptr->bitmap)
				{
					byte_ptr tmp = (byte_ptr)((uintptr_t)ptr + ptr->total_size(8, (byte_ptr)ptr) - (uintptr_t)ptr->chunk_type * chunk_size);
					uintptr_t offset = ((uintptr_t)to_dealloc - (uintptr_t)tmp);
					offset = offset / ptr->chunk_type;

					clear_bits((byte_ptr)ptr, offset, n);
					tmp = (byte_ptr)ptr + Chunk::size();
					if (check_for_empty_bmp_chunk(tmp))
					{
						get_rid_of_chunk(ptr);
					}
					return;
				}
				else
				{
					get_rid_of_chunk(ptr);
					return;
				}
			}
			ptr = ptr->next;
		}
		//should never happen!
	}

	template<typename U>
	bool operator == (const inblock_allocator<U, HeapHolder> &)
	{
		return true;
	}

	template<typename U>
	bool operator != (const inblock_allocator<U, HeapHolder> &)
	{
		return false;
	}
private:

	// Takes out the chunk from linked list
	void get_rid_of_chunk(Chunk* ptr)
	{
		if (!ptr->previous && !ptr->next)
			HeapHolder::heap.first_chunk = nullptr;
		if (ptr->previous)
			ptr->previous->next = ptr->next;
		else
			HeapHolder::heap.first_chunk = ptr->next;
		if (ptr->next)
			ptr->next->previous = ptr->previous;
	}

	// Checks if bitmap is empty
	bool check_for_empty_bmp_chunk(byte_ptr ptr)
	{
		for (size_t i = 0; i < chunk_size / 8; ++i)
		{
			if (*(ptr + i) != (uint8_t)0U)
				return false;
		}
		return true;
	}

	// Finds free space of given magnitude in bitmap
	pointer find_free_space(byte_ptr start, size_t bitmap_size, size_t chunk_type_size, size_t n)
	{
		size_t position = 0;
		size_t empty_counter = 0;
		size_t pos_i = 0;
		size_t pos_bit_offset = 0;
		for (size_t i = 0; i < bitmap_size; ++i)
		{
			for (size_t bit_offset = 0; bit_offset < 8; ++bit_offset)
			{
				if (((*(start + i) >> bit_offset) & (uint8_t)1U) == 0)
				{
					if (empty_counter == 0)
					{
						pos_i = i;
						pos_bit_offset = bit_offset;
					}
					++empty_counter;
					if (empty_counter == n)
					{
						return set_bits(start, pos_i, pos_bit_offset, chunk_type_size, n, bitmap_size, position);
					}
				}
				else
				{
					pos_bit_offset = 0;
					pos_i = 0;
					position += empty_counter + 1;
					empty_counter = 0;
				}
			}
		}
		return nullptr;
	}

	// Clears bits in the given bitmap
	void clear_bits(byte_ptr ptr, size_t offset, size_t n)
	{
		byte_ptr tmp = (byte_ptr)ptr + Chunk::size();
		tmp = tmp + offset / 8;
		size_t bits = offset % 8;
		size_t counter = 0;
		size_t cap = (n + bits > 8) ? 8 : n + bits;
		for (size_t j = bits; j < cap; ++j)
		{
			*(tmp) = *(tmp) & ((uint8_t)255U - (uint8_t)(1U << j));
			++counter;
		}
		cap = counter;
		if (n - cap > 8)
			for (size_t j = 0; j < (n - cap) / 8; ++j)
			{
				++tmp;
				*tmp = (uint8_t)0U;
				counter += 8;
			}
		cap = counter;
		++tmp;
		for (size_t j = 0; j < n - cap; j++)
		{
			*(tmp) = *(tmp) & ((uint8_t)255U - (uint8_t)(1U << j));
			++counter;
		}
	}

	// Sets bits in the given bitmap
	pointer set_bits(byte_ptr start, size_t pos_i, size_t pos_bit_offset, size_t chunk_type_size, size_t n, size_t bitmap_size, size_t position)
	{
		size_t counter = 0;
		size_t cap = (n + pos_bit_offset > 8) ? 8 : (n + pos_bit_offset);
		for (size_t j = pos_bit_offset; j < cap; ++j)
		{
			*(start + pos_i) = (*(start + pos_i)) | (1 << j);
			++counter;
		}
		cap = counter;
		if (n - cap > 8)
		{
			for (size_t k = 0; k < (n - cap) / 8; ++k)
			{
				++pos_i;
				*(start + pos_i) = (uint8_t)255U;
				counter += 8;
			}
		}
		++pos_i;
		cap = counter;
		for (size_t j = 0; j < n - cap; ++j)
		{
			*(start + pos_i) = (*(start + pos_i)) | (1 << j);
		}
		uintptr_t align = (uintptr_t)(start)+bitmap_size;
		align = 8 - align % 8;
		return (pointer)(start + bitmap_size + align + position * chunk_type_size);
	}

	// Counts the chunk size without any alignemnt that could occur inside
	size_t count_chunk_size_no_align(size_t size_type)
	{
		return Chunk::size() + chunk_size / 8 + chunk_size * size_type;
	}

	// Creates bitmap chunk and puts it in linked list
	pointer init_bmp_chunk(size_t size_type, size_t n, Chunk * next, Chunk* prev, byte_ptr address)
	{
		Chunk* chunk = new ((Chunk*)address) Chunk(next, prev, size_type, true);
		for (size_t i = 0; i < chunk_size / 8; ++i)      //clear bmp
		{
			*(((byte_ptr)chunk) + i + Chunk::size()) = 0;
		}
		if (!next && !prev)
			HeapHolder::heap.first_chunk = chunk;
		if (next)
			next->previous = chunk;
		if (prev)
			prev->next = chunk;
		else
			HeapHolder::heap.first_chunk = chunk;
		return find_free_space(((byte_ptr)chunk) + Chunk::size(), chunk_size / 8, size_type, n);
	}

	// Returns number to add to given address to get aligned address
	size_t get_alignment(byte_ptr ptr)
	{
		size_t align = (size_t)ptr;
		align = 8 - align % 8;
		return align;
	}

	// Looks for space in the memory for new bitmap chunk and initializes it
	pointer insert_new_bmp_chunk(size_t size_type, size_t n)
	{
		Chunk * ptr = HeapHolder::heap.first_chunk;
		size_t adr = get_alignment(HeapHolder::heap.heap_ptr);
		// No chunk is in memory
		if (ptr == nullptr)
		{
			return init_bmp_chunk(size_type, n, nullptr, nullptr, HeapHolder::heap.heap_ptr + adr);
		}

		// Checks if there is enough space for a chunk between the first chunk and "start" of memory
		adr = get_alignment(HeapHolder::heap.heap_ptr);
		if (((uintptr_t)ptr - (uintptr_t)HeapHolder::heap.heap_ptr) > size_type + adr + count_chunk_size_no_align(size_type) && valid_ptr((byte_ptr)ptr))
		{
			return init_bmp_chunk(size_type, n, ptr, nullptr, HeapHolder::heap.heap_ptr + adr);
		}

		// Checks if there is enough space for a chunk between two chunks
		Chunk * next = ptr->next;
		Chunk* end = (Chunk *)(((byte_ptr)ptr) + ptr->total_size(8, (byte_ptr)ptr));
		while (next)
		{
			adr = get_alignment((byte_ptr)end);// (size_t)(end) % 8;
			if (((uintptr_t)next - (uintptr_t)end) > size_type + adr + count_chunk_size_no_align(size_type)
				&& valid_ptr((byte_ptr)end) && valid_ptr((byte_ptr)next) && valid_ptr((byte_ptr)ptr))
			{
				return init_bmp_chunk(size_type, n, next, ptr, ((byte_ptr)end) + adr);
			}
			ptr = next;
			next = ptr->next;
			end = (Chunk *)(((byte_ptr)ptr) + ptr->total_size(8, (byte_ptr)ptr));
		}

		// Checks if there is enough space for a chunk between the last chunk and the "end" of memory
		adr = get_alignment((byte_ptr)end);
		if (valid_ptr((byte_ptr)end) && ((uintptr_t)HeapHolder::heap.heap_ptr + HeapHolder::heap.heap_size - ((uintptr_t)end))
			> adr + size_type + count_chunk_size_no_align(size_type))
		{
			return init_bmp_chunk(size_type, n, nullptr, ptr, ((byte_ptr)end) + adr);
		}
		// No space for new chunk found
		throw std::bad_alloc();
	}

	// Checks if the given pointer is within boundaries of given memory
	bool valid_ptr(byte_ptr ptr)
	{
		return (ptr == nullptr || ((uintptr_t)(HeapHolder::heap.heap_ptr) <= (uintptr_t)ptr && (uintptr_t)ptr <= (uintptr_t)(HeapHolder::heap.heap_ptr + HeapHolder::heap.heap_size))) ? true : false;
	}

	// Creates big chunk and puts it in linked list
	pointer init_big_chunk(size_t size_type, size_t n, Chunk * next, Chunk* prev, byte_ptr address)
	{
		Chunk* chunk = new ((Chunk *)address) Chunk(next, prev, size_type * n, false);
		if (next)
			next->previous = chunk;
		if (!next && !prev)
			HeapHolder::heap.first_chunk = chunk;
		if (prev)
			prev->next = chunk;
		else
			HeapHolder::heap.first_chunk = chunk;
		uintptr_t align = ((uintptr_t)chunk + Chunk::size());
		align = 8 - align % 8;
		return (pointer)(((byte_ptr)chunk) + Chunk::size() + align);
	}

	// Looks for space in the memory for new bitmap chunk and initializes it
	pointer insert_new_big_chunk(size_t size_type, size_t n)
	{
		Chunk * ptr = HeapHolder::heap.first_chunk;
		size_t adr = get_alignment(HeapHolder::heap.heap_ptr);

		// No chunk is in memory
		if (!ptr)
		{
			return init_big_chunk(size_type, n, nullptr, nullptr, (HeapHolder::heap.heap_ptr) + adr);
		}

		// Checks if there is enough space for a chunk between the first chunk and "start" of memory
		adr = get_alignment(HeapHolder::heap.heap_ptr);
		if (((uintptr_t)ptr - (uintptr_t)HeapHolder::heap.heap_ptr) > 8 + size_type + Chunk::size() + adr + size_type * n && valid_ptr((byte_ptr)ptr))
		{
			return init_big_chunk(size_type, n, ptr, nullptr, HeapHolder::heap.heap_ptr + adr);
		}

		// Checks if there is enough space for a chunk between two chunks
		Chunk * next = ptr->next;
		Chunk * end = (Chunk *)(((byte_ptr)ptr) + ptr->total_size(8, (byte_ptr)ptr));
		while (next)
		{
			adr = get_alignment((byte_ptr)end);
			if (((uintptr_t)next - (uintptr_t)end) > 8 + size_type + Chunk::size() + adr + size_type * n
				&& valid_ptr((byte_ptr)end) && valid_ptr((byte_ptr)next) && valid_ptr((byte_ptr)ptr))
			{
				return init_big_chunk(size_type, n, next, ptr, ((byte_ptr)end) + adr);
			}
			ptr = ptr->next;
			next = ptr->next;
			end = (Chunk *)(((byte_ptr)ptr) + ptr->total_size(8, (byte_ptr)ptr));
		}

		// Checks if there is enough space for a chunk between the last chunk and the "end" of memory
		adr = get_alignment((byte_ptr)end);
		if (valid_ptr((byte_ptr)end) && ((uintptr_t)HeapHolder::heap.heap_ptr + HeapHolder::heap.heap_size - ((uintptr_t)end)) > 8 + size_type + Chunk::size() + adr + size_type * n)
		{
			return init_big_chunk(size_type, n, nullptr, ptr, (byte_ptr)(end)+adr);
		}
		// No space for new chunk found
		throw std::bad_alloc();
	}

	// Returns the next 2^x greater the sizeof(value_type)
	size_t determine_correct_chunk_size()
	{
		size_t tmp = (sizeof(value_type));
		size_t result = 1;
		while (tmp > result)
		{
			result = result << 1;
		}
		return result;
	}
};
#endif // !INBLOCK_ALLOCATOR
