#pragma once
#include <windows.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>
#include"suspender.h"

//原本64位也能用，但copilot不推荐
namespace utils {

	using uint = unsigned int;

	inline void**& getvtable(void* inst, std::size_t offset = 0) {
		return *reinterpret_cast<void***>(reinterpret_cast<std::uint8_t*>(inst) + offset);
	}

	// 非强依赖 VirtualQuery 的版本（可选：你也可以保留原来的 CountFuncs）
	inline uint CountFuncs(void** vmt, uint max_scan = 512) {
		uint i = 0;
		MEMORY_BASIC_INFORMATION mbi{};
		while (i < max_scan) {
			if (!VirtualQuery(vmt[i], &mbi, sizeof(mbi)))
				break;

			const auto prot = mbi.Protect & 0xFF;
			if (prot != PAGE_EXECUTE &&
				prot != PAGE_EXECUTE_READ &&
				prot != PAGE_EXECUTE_READWRITE &&
				prot != PAGE_EXECUTE_WRITECOPY)
				break;

			++i;
		}
		return i;
	}


	//只要是“类内定义”的成员函数，标准就规定它们自动是 inline 的，这里写法冗余了
	class VtableHook {
	public:
		VtableHook(void* inst, std::size_t offset = 0, uint vfuncs = 0)
			: vtable_ptr_(&getvtable(inst, offset)),
			original_vtable_(*vtable_ptr_),
			hooked_vtable_(nullptr),
			vtable_function_count_(0) {

			if (!vfuncs)
				vfuncs = CountFuncs(original_vtable_);

			vtable_function_count_ = vfuncs;

			hooked_vtable_ = static_cast<void**>(
				std::malloc(sizeof(void*) * vfuncs)
				);
			std::memcpy(hooked_vtable_, original_vtable_, sizeof(void*) * vfuncs);

			//*vtable_ptr_ = hooked_vtable_;
			//换成更安全的原子交互，防止指针撕裂
			Suspender suspend;
			InterlockedExchangePointer(
				reinterpret_cast<PVOID*>(vtable_ptr_),
				hooked_vtable_);
		}

		~VtableHook() {
			if (vtable_ptr_)
				Unhook();
			std::free(hooked_vtable_);
		}

		inline void HookMethod(void* newfunc, std::size_t index) {
			assert(index < vtable_function_count_);
			//hooked_vtable_[index] = newfunc;
			Suspender suspend;
			InterlockedExchangePointer(
				reinterpret_cast<PVOID*>(&hooked_vtable_[index]),
				newfunc);
		}

		inline void UnhookMethod(std::size_t index) {
			assert(index < vtable_function_count_);
			//hooked_vtable_[index] = original_vtable_[index];
			Suspender suspend;
			InterlockedExchangePointer(
				reinterpret_cast<PVOID*>(&hooked_vtable_[index]),
				original_vtable_[index]);
		}

		inline void Unhook() {
			//*vtable_ptr_ = original_vtable_;
			Suspender suspend;
			InterlockedExchangePointer(
				reinterpret_cast<PVOID*>(vtable_ptr_),
				original_vtable_);
		}
		inline void Rehook() {
			//*vtable_ptr_ = hooked_vtable_;
			Suspender suspend;
			InterlockedExchangePointer(
				reinterpret_cast<PVOID*>(vtable_ptr_),
				hooked_vtable_);
		}
		inline bool Hooked() const { return *vtable_ptr_ == hooked_vtable_; }

		inline uint NumFuncs() const { return vtable_function_count_; }

		inline void Poof() { vtable_ptr_ = nullptr; }

		template<typename Fn>
		inline Fn GetMethod(std::size_t index) const {
			assert(index < vtable_function_count_);
			return reinterpret_cast<Fn>(original_vtable_[index]);
		}

	private:
		void*** vtable_ptr_;       // &obj->vfptr
		void** original_vtable_;  // 原始 vtable
		void** hooked_vtable_;    // 克隆后的 vtable
		uint    vtable_function_count_;
	};

} // namespace utils
