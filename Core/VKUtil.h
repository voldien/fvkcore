/*
 * Copyright (c) 2021 Valdemar Lindberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _FVK_VK_UTILS_H_
#define _FVK_VK_UTILS_H_ 1
#include "Exception.hpp"
#include <vulkan/vulkan.h>

#ifndef FVK_SCH_EXTERN
#ifndef FVK_SCH_STATIC
#ifdef _WIN32
#define FVK_SCH_EXTERN __declspec(dllimport)
#elif defined(__GNUC__) && __GNUC__ >= 4 || __clang__
#define FVK_SCH_EXTERN __attribute__((visibility("default")))
#else
#define FVK_SCH_EXTERN
#endif
#else
#define FVK_SCH_EXTERN
#endif
#endif

/**
 * @brief
 *
 * @param symbol
 * @return const char*
 */
extern "C" FVK_SCH_EXTERN const char *getVKResultSymbol(int symbol);

#define VKS_VALIDATE(x)                                                                                                \
	do {                                                                                                               \
		VkResult _err = x;                                                                                             \
		if (_err != VK_SUCCESS) {                                                                                      \
			throw cxxexcept::RuntimeException("{} {} {} - {}", __FILE__, __LINE__, _err, getVKResultSymbol(_err));     \
		}                                                                                                              \
	} while (0)

#endif
