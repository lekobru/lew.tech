#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <cmath>
#include <Psapi.h>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace global_utils {

	template <typename T>
	static T* get_interface(const char* szModule, const char* szInterface) {

		typedef void* (*o_interface_t)(const char*, int*);
		o_interface_t original = (o_interface_t)GetProcAddress(GetModuleHandleA(szModule), "CreateInterface");
		return (T*)original(szInterface, nullptr);

	}

	void* find_interface(const char* module, const char* interface_name);

	template <typename T>
	static T v_function(void* pClass, int iIndex) {

		PDWORD p_vtable = *(PDWORD*)pClass;
		DWORD dw_address = p_vtable[iIndex];

		return (T)(dw_address);

	}
}