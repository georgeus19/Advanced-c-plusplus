// concurrent_bitmap.hpp
// <Krystof Hruby> NPRG051 2019/2020 
#ifndef BITMAP
#define BITMAP
#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <array>

struct node
{
	virtual ~node() {}
};

struct inner_node : public node
{
	std::array<node*, 64> data;
	inner_node();
	~inner_node();
};

struct leaf : public node
{
	std::array<std::atomic_uint8_t, 2048> data;
	leaf();
	~leaf();
};

class concurrent_bitmap
{
public:

	using key_type = uint32_t;
	using value_type = bool;

		// Return value stored at key address
	value_type get(key_type key) const;

		// Set value at the address corresponding to key
	void set(key_type key, value_type value);

	concurrent_bitmap(const concurrent_bitmap& other) = delete;
	concurrent_bitmap(concurrent_bitmap&& other);

	concurrent_bitmap& operator =(const concurrent_bitmap& other) = delete;
	concurrent_bitmap& operator =(concurrent_bitmap&& other);

	concurrent_bitmap();
	~concurrent_bitmap();
private:
	node* root;
	std::mutex mtx;
	void swap(concurrent_bitmap& other);

		// Returns the value that is stored in key from index `from` to index `to`
	size_t get_offset(size_t from, size_t to, key_type key) const;

		// Allocates inner node and stores the pointer in the inner node ptr points to at given offset
	node * create_inner_node(node * ptr, key_type offset);

		// Allocates leaf node and stores the pointer in the inner node ptr points to at given offset
	node * create_leaf_node(node * ptr, key_type offset);

		// Creates two inner nodes and one leaf node that serve as path to the address corresponding to key
	bool create_three_nodes(node * ptr, key_type offset, key_type key, value_type value);
		// Creates one inner node and one leaf node that serve as path to the address corresponding to key
	bool create_two_nodes(node * ptr, key_type offset, key_type key, value_type value);
		// Sets the bit to value at the address corresponding to key
	void set_bit(value_type value, key_type key, node*ptr);
	
	node * alloc_inner_node();
	node * alloc_leaf_node();
};

#endif // BITMAP
