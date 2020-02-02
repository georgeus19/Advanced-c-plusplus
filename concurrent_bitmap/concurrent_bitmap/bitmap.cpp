#include "pch.h"
#include <string>
#include <iostream>
#include <thread>
#include "concurrent_bitmap.h"

using namespace std;

bool run_test(size_t thread_count, size_t address_base, size_t thread_byte_offset, size_t tested_length)
{
	concurrent_bitmap cbmp;

	thread* threads = new thread[thread_count];
	for (size_t thread_id = 0; thread_id < thread_count; thread_id++)
	{
		threads[thread_id] = thread([=, &cbmp]()
		{
			for (size_t i = 0; i < tested_length; i++)
			{
				// Each thread sets a bit in a particular byte according to its index
				size_t bit_index = address_base + thread_byte_offset * thread_id + i * 8 + thread_id;
				//std::cout << hex << bit_index << "bit index zapis\n";
				cbmp.set(bit_index, true);
				//std::cout << cbmp.get(bit_index) << "getindexALLWAYS TRUE\n";
			}
		});
	}

	for (size_t i = 0; i < thread_count; i++)
	{
		threads[i].join();
	}

	for (size_t i = 0; i < tested_length; i++)
	{
		for (size_t thread_id = 0; thread_id < thread_count; thread_id++)
		{
			// Check that the bit was written
			size_t bit_index = address_base + thread_byte_offset * thread_id + i * 8 + thread_id;
			//std::cout << bit_index << "bit index read\n";
			if (!cbmp.get(bit_index))
			{
				cout << "Error at bit " << hex << bit_index << endl;
				return false;
			}
		}
	}

	delete[] threads;

	cout << "Test run OK" << endl;
	return true;
}

void f(concurrent_bitmap&  x)
{

}

int main(int argc, char** argv)
{
	//{
	//	//pointer = (T*)operator new (sizeof(T)*(other.numberOfElements));
		//uintptr_t** root = (uintptr_t **)operator new (sizeof(uintptr_t *) * 64);
		//for (size_t i = 0; i < 64; i++)
		//{
		//	new (root + i) uintptr_t*(nullptr);
		//}
		//for (size_t i = 0; i < 64; i++)
		//{
		//	std::cout << *(root + i) << std::endl;
		//	if (*(root+i) == nullptr)
		//		std::cout << "NULLPTR " << i << std::endl;
		//}
		//concurrent_bitmap cbmp;
		//size_t thread_count = stoul(argv[1]);
		//size_t repeat_count = stoul(argv[2]);
		//size_t address_base = stoul(argv[3], nullptr, 16);
		//size_t thread_byte_offset = stoul(argv[4], nullptr, 16);
		//size_t tested_length = stoul(argv[5], nullptr, 16);
		//size_t b = address_base + thread_byte_offset * 0 + 0 * 8 + 0;
		//for (size_t i = 0; i < tested_length; i++)
		//{
		//	if (i == 1984)
		//	{
		//		std::cout << "!";
		//	}
		//	// Each thread sets a bit in a particular byte according to its index
		//	size_t bit_index = address_base + thread_byte_offset * 0 + i * 8 + 0;
		//	//std::cout << hex << bit_index << "bit index zapis\n";
		//	cbmp.set(bit_index, true);
		//	//std::cout << cbmp.get(bit_index) << "getindexALLWAYS TRUE\n";
		//	if (!cbmp.get(bit_index))
		//	{
		//		cout << "Error at bit 1-" << hex << bit_index << endl;
		//	}
		//	if (!cbmp.get(b))
		//	{
		//		cout << i << "Error at bit " << hex << bit_index << endl;
		//	}
		//}
		//
		//
		//if (!cbmp.get(b))
		//{
		//	cout << "Error at bit 2-" << hex << b << endl;
		//}
		//
		//for (size_t i = 0; i < tested_length; i++)
		//{
		//
		//	// Check that the bit was written
		//	size_t bit_index = address_base + thread_byte_offset * 0 + i * 8 + 0;
		//	//std::cout << bit_index << "bit index read\n";
		//	if (!cbmp.get(bit_index))
		//	{
		//		cout << "Error at bit 3-" << hex << bit_index << endl;
		//		return false;
		//	}
		//
		//}
		//
	//}
	//concurrent_bitmap bmp;
	////auto x = move(bmp);
	//f(bmp);
	//bmp.set(0, true);
	//bmp.set(1, true);
	//bmp.set(0, false);
	//std::cout << bmp.get(0) << " ";
	//std::cout << bmp.get(1) << " ";
	//std::cout << bmp.get(1000000) << " ";
	//
	//bmp.set(1111638528, true);
	//bmp.set(1111638530, false);
	//bmp.set(1111638529, false);
	//bmp.set(1111638527, false);
	//bmp.set(1111638529, true);
	//std::cout << bmp.get(1111638528) << " ";
	size_t thread_count = stoul(argv[1]);
	size_t repeat_count = stoul(argv[2]);
	size_t address_base = stoul(argv[3], nullptr, 16);
	size_t thread_byte_offset = stoul(argv[4], nullptr, 16);
	size_t sample_count = stoul(argv[5], nullptr, 16);
	
	for (size_t i = 0; i < repeat_count; i++)
	{
		if (!run_test(thread_count, address_base, thread_byte_offset, sample_count))
		{
			cout << "Error" << endl;
			return 0;
		}
	}

	cout << "OK" << endl;

	return 0;
}