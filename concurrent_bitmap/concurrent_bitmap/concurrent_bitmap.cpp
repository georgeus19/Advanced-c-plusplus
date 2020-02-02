// concurrent_bitmap.hpp
// <Krystof Hruby> NPRG051 2019/2020 
#include "pch.h"
#include "concurrent_bitmap.h"

using value_type = concurrent_bitmap::value_type;
using key_type = concurrent_bitmap::key_type;

inner_node::inner_node()
{
	data.fill(nullptr);
}

leaf::leaf()
{
	for (auto && x : data)
	{
		x = { 0 };
	}
}

inner_node::~inner_node()
{
	for (size_t i = 0; i < data.size(); ++i)
	{
		delete data[i];
	}
}

leaf::~leaf() {}

concurrent_bitmap::~concurrent_bitmap()
{
	delete root;
}

void concurrent_bitmap::swap(concurrent_bitmap & other)
{
	std::swap(root, other.root);
}

concurrent_bitmap::concurrent_bitmap(concurrent_bitmap&& other)
{
	root = other.root;
}

concurrent_bitmap& concurrent_bitmap::operator =(concurrent_bitmap&& other)
{
	swap(other);
	return *this;
}

	// Returns the value that is stored in key from index `from` to index `to`
size_t concurrent_bitmap::get_offset(size_t from, size_t to, key_type key) const
{
	key_type mask = ((1 << (to - from)) - 1) << from;
	return (key & mask) >> from;
}

	// Return value stored at key address
	// If the child of inner_node is not allocated then the values at the addresses in question are false
value_type concurrent_bitmap::get(key_type key) const
{
	key_type offset = get_offset(26, 32, key);
	if (!((static_cast<inner_node*>(root))->data[offset]))
		return false;

	node* ptr = (static_cast<inner_node*>(root))->data[offset];
	offset = get_offset(20, 26, key);
	if (!((static_cast<inner_node*>(ptr))->data[offset]))
		return false;

	ptr = (static_cast<inner_node*>(ptr))->data[offset];
	offset = get_offset(14, 20, key);
	if (!((static_cast<inner_node*>(ptr))->data[offset]))
		return false;

		// All nodes in path are allocated. Returning the value stored at given address
	ptr = (static_cast<inner_node*>(ptr))->data[offset];
	offset = get_offset(3, 14, key);
	uint8_t byte = (static_cast<leaf*>(ptr))->data[offset];
	offset = get_offset(0, 3, key);
	return ((byte >> offset) % 2 == 1) ? true : false;
}

	// Set value at the address corresponding to key
	// Not much expandable but less error prone than recursive set and more clear.
	// For each tree level check is done if there is a valid pointer to node corresponding to the key address
	// If value is false and the path to leaf in not allocated, there is no need to actually allocate any node, since it's set to false in default
void concurrent_bitmap::set(key_type key, value_type value)
{
	key_type offset = get_offset(26, 32, key);
	if (!((static_cast<inner_node*>(root))->data[offset]))
	{
			// the whole path in not allocated
		if (create_three_nodes(root, offset, key, value))
			return;
	}
		
	node* ptr = (static_cast<inner_node*>(root))->data[offset];
	offset = get_offset(20, 26, key);
	if (!((static_cast<inner_node*>(ptr))->data[offset]))
	{
			// the leaf and his parent is not allocated
		if (create_two_nodes(ptr, offset, key, value))
			return;
	}
		
	ptr = (static_cast<inner_node*>(ptr))->data[offset];
	offset = get_offset(14, 20, key);
	if (!((static_cast<inner_node*>(ptr))->data[offset]))
	{
			// the leaf in not allocated
		std::lock_guard<std::mutex> lock(mtx);
		if (!(static_cast<inner_node*>(ptr)->data[offset]))
		{
			ptr = create_leaf_node(ptr, get_offset(14, 20, key));
			set_bit(value, key, ptr);
			return;
		}
	}

		// All nodes allocated, just setting the value in the leaf
	ptr = (static_cast<inner_node*>(ptr))->data[offset];
	set_bit(value, key, ptr);
}

	// Sets the bit to value at the address corresponding to key
void concurrent_bitmap::set_bit(value_type value, key_type key, node* ptr)
{
	if (value)
	{
		uint8_t add_value = (1 << get_offset(0, 3, key));
		static_cast<leaf*>(ptr)->data[get_offset(3, 14, key)] |= add_value;
	}
	else
	{
		uint8_t add_value = ((1 << 8) - 1) - (1 << get_offset(0, 3, key));
		static_cast<leaf*>(ptr)->data[get_offset(3, 14, key)] &= add_value;
	}
}

	// Allocates inner node and stores the pointer in the inner node ptr points to at given offset
node * concurrent_bitmap::create_inner_node(node * ptr, key_type offset)
{
	(static_cast<inner_node*>(ptr))->data[offset] = alloc_inner_node();
	return (static_cast<inner_node*>(ptr))->data[offset];
}

	// Allocates leaf node and stores the pointer in the inner node ptr points to at given offset
node * concurrent_bitmap::create_leaf_node(node * ptr, key_type offset)
{
	(static_cast<inner_node*> (ptr))->data[offset] = alloc_leaf_node();
	return (static_cast<inner_node*>(ptr))->data[offset];
}

	// Creates two inner nodes and one leaf node that serve as path to the address corresponding to key
bool concurrent_bitmap::create_three_nodes(node * ptr, key_type offset, key_type key, value_type value)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (!(static_cast<inner_node*>(ptr)->data[offset]))
	{
		if (value == false)
			return true;
		ptr = create_inner_node(ptr, offset);
		ptr = create_inner_node(ptr, get_offset(20, 26, key));
		ptr = create_leaf_node(ptr, get_offset(14, 20, key));
		set_bit(value, key, ptr);
		return true;
	}
	return false;
}

	// Creates one inner node and one leaf node that serve as path to the address corresponding to key
bool concurrent_bitmap::create_two_nodes(node * ptr, key_type offset, key_type key, value_type value)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (!(static_cast<inner_node*>(ptr)->data[offset]))
	{
		if (value == false)
			return true;
		ptr = create_inner_node(ptr, offset);
		ptr = create_leaf_node(ptr, get_offset(14, 20, key));
		set_bit(value, key, ptr);
		return true;
	}
	return false;
}

node * concurrent_bitmap::alloc_leaf_node()
{
	return (leaf*)new leaf;
}

node * concurrent_bitmap::alloc_inner_node()
{
	return (inner_node*)new inner_node;
}

concurrent_bitmap::concurrent_bitmap()
{
	root = alloc_inner_node();
}