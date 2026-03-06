#pragma once
#include <string>
#include <unordered_map>
#include <v8.h>

class ModuleLoader {
public:
	explicit ModuleLoader(v8::Isolate* isolate);

	// 加载模块（文件名 → Module）
	v8::MaybeLocal<v8::Module> LoadModule(const std::wstring& filename);

	// 解析 import
	static v8::MaybeLocal<v8::Module> ResolveCallback(
		v8::Local<v8::Context> context,
		v8::Local<v8::String> specifier,
		v8::Local<v8::FixedArray> import_attributes,
		v8::Local<v8::Module> referrer);
	// 动态 import() 回调
	static v8::MaybeLocal<v8::Promise> ImportModuleDynamicallyCallback(
		v8::Local<v8::Context> context,
		v8::Local<v8::Data> host_defined_options,
		v8::Local<v8::Value> resource_name,
		v8::Local<v8::String> specifier,
		v8::Local<v8::FixedArray> import_attributes);


	// 实例化 + 执行
	bool InstantiateAndEvaluate(v8::Local<v8::Module> module);

	bool LoadDomain(const std::wstring& filename);

private:
	v8::Isolate* isolate_;

	// 模块缓存：identity_hash → Module
	std::unordered_map<int, v8::Global<v8::Module>> module_cache_;

	// 文件名缓存：identity_hash → filename
	std::unordered_map<int, std::wstring> module_filenames_;

	// 读取文件
	std::string ReadFile(const std::wstring& filename);

	// 注册到缓存
	void CacheModule(v8::Local<v8::Module> module, const std::wstring& filename);
};
