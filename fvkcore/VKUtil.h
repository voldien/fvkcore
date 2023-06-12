
#pragma once
#include <Exception.hpp>
#include <vulkan/vulkan.h>

#ifndef FVK_DECL_EXTERN
#ifndef FVK_SCH_STATIC
#ifdef _WIN32
#define FVK_DECL_EXTERN __declspec(dllimport)
#elif defined(__GNUC__) && __GNUC__ >= 4 || __clang__
#define FVK_DECL_EXTERN __attribute__((visibility("default")))
#else
#define FVK_DECL_EXTERN
#endif
#else
#define FVK_DECL_EXTERN
#endif
#endif
namespace fvkcore {

	/**
	 * @brief
	 *
	 * @param symbol
	 * @return const char*
	 */
	extern FVK_DECL_EXTERN const char *getVKResultSymbol(int symbol);

} // namespace fvkcore

#define VKS_VALIDATE(x)                                                                                                \
	do {                                                                                                               \
		VkResult _err = x;                                                                                             \
		if (_err != VK_SUCCESS) {                                                                                      \
			throw cxxexcept::RuntimeException("{} {} {} - {}", __FILE__, __LINE__, (size_t)_err,                       \
											  fvkcore::getVKResultSymbol(_err));                                       \
		}                                                                                                              \
	} while (0)



