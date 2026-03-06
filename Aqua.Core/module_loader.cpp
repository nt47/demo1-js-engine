#include "module_loader.h"
#include "utils.h"
#include"encoding.h"
#include"system.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace v8;
using namespace utils;


ModuleLoader::ModuleLoader(Isolate* isolate)
	: isolate_(isolate) {
	// 绑定到 isolate 的 slot 0 
	isolate_->SetData(0, this);
}

std::string ModuleLoader::ReadFile(const std::wstring& filename) {
	std::ifstream f(filename);
	if (!f.is_open()) return "";
	std::stringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

void ModuleLoader::CacheModule(Local<Module> module, const std::wstring& filename) {
	int id = module->GetIdentityHash();
	module_cache_[id].Reset(isolate_, module);
	module_filenames_[id] = filename;
}

MaybeLocal<Module> ModuleLoader::LoadModule(const std::wstring& filename) {
	// 缓存命中
	for (auto& [id, mod] : module_cache_) {
		if (module_filenames_[id] == filename) {
			return mod.Get(isolate_);
		}
	}
	//System::PrintLn(LEVEL::DEBUG, "Directory: {}", GetModuleDirectoryUtf8());
	auto full_name = GetModuleDirectoryW() + +L"scripts\\" + filename;

	std::string source_code = ReadFile(full_name);
	if (source_code.empty()) {
		std::cerr << "Cannot read file: " << w2s(filename) << std::endl;
		return MaybeLocal<Module>();
	}

	Local<String> source_text =
		String::NewFromUtf8(isolate_, source_code.c_str()).ToLocalChecked();

	Local<String> resource_name =
		String::NewFromUtf8(isolate_, w2s(filename).c_str()).ToLocalChecked();

	ScriptOrigin origin(
		resource_name,
		0, 0, false, -1,
		Local<Value>(), false, false, true, Local<Data>()
	);

	ScriptCompiler::Source source(source_text, origin);

	Local<Module> module =
		ScriptCompiler::CompileModule(isolate_, &source).ToLocalChecked();

	CacheModule(module, filename);

	return module;
}

static ModuleLoader* GetLoaderFromContext() {
	Isolate* isolate = Isolate::GetCurrent();
	void* data = isolate->GetData(0);
	return static_cast<ModuleLoader*>(data);
}


MaybeLocal<Module> ModuleLoader::ResolveCallback(
	Local<Context> context,
	Local<String> specifier,
	Local<FixedArray> import_attributes,
	Local<Module> referrer) {

	v8::Isolate* isolate = v8::Isolate::GetCurrent();

	ModuleLoader* g_loader = GetLoaderFromContext();//新增
	if (!g_loader) {
		std::cerr << "ModuleLoader not found in isolate data" << std::endl;
		return MaybeLocal<Module>();
	}

	String::Utf8Value spec(isolate, specifier);

	int id = referrer->GetIdentityHash();
	auto it = g_loader->module_filenames_.find(id);
	if (it == g_loader->module_filenames_.end()) {
		std::cerr << "Referrer filename not found" << std::endl;
		return MaybeLocal<Module>();
	}

	std::wstring ref_name = it->second;

	size_t pos = ref_name.find_last_of('/');
	std::wstring dir = (pos == std::wstring::npos)
		? L""
		: ref_name.substr(0, pos + 1);

	std::wstring resolved = dir + s2w(*spec);

	return g_loader->LoadModule(resolved);
}

bool ModuleLoader::InstantiateAndEvaluate(Local<Module> module) {
	Local<Context> context = isolate_->GetCurrentContext();

	if (module->InstantiateModule(context, ResolveCallback).IsNothing()) {
		std::cerr << "Instantiate failed" << std::endl;
		return false;
	}

	MaybeLocal<Value> result = module->Evaluate(context);//执行并返回结果
	if (result.IsEmpty()) {
		std::cerr << "Evaluate failed" << std::endl;
		return false;
	}

	return true;
}

// 简单的错误帮助函数
static void RejectWithError(Local<Context> context,
	Local<Promise::Resolver> resolver,
	const std::string& message) {

	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	Local<String> msg =
		String::NewFromUtf8(isolate, message.c_str()).ToLocalChecked();
	Local<Value> error = Exception::Error(msg);
	resolver->Reject(context, error).ToChecked();
}

MaybeLocal<Promise> ModuleLoader::ImportModuleDynamicallyCallback(
	Local<Context> context,
	Local<Data> host_defined_options,
	Local<Value> resource_name,
	Local<String> specifier,
	Local<FixedArray> import_attributes) {

	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	EscapableHandleScope handle_scope(isolate);

	// 1. 创建 Promise 和 Resolver
	Local<Promise::Resolver> resolver;
	if (!Promise::Resolver::New(context).ToLocal(&resolver)) {
		return MaybeLocal<Promise>();
	}
	Local<Promise> promise = resolver->GetPromise();


	ModuleLoader* g_loader = GetLoaderFromContext();//新增
	if (!g_loader) {
		RejectWithError(context, resolver, "ModuleLoader not found in isolate data");
		return handle_scope.Escape(promise);
	}

	// 2. specifier 字符串
	String::Utf8Value spec_utf8(isolate, specifier);
	std::string spec_str = *spec_utf8 ? *spec_utf8 : "";

	// 3. 获取 referrer 的文件名（resource_name）
	if (!resource_name->IsString()) {
		RejectWithError(context, resolver, "Dynamic import missing resource_name");
		return handle_scope.Escape(promise);
	}

	String::Utf8Value ref_utf8(isolate, resource_name.As<String>());
	std::wstring ref_filename = *ref_utf8 ? s2w(*ref_utf8) : L"";

	// 4. 找到 referrer 的 Module（从缓存）
	Local<Module> ref_module;
	{
		bool found = false;
		for (auto& [id, mod] : g_loader->module_cache_) {
			if (g_loader->module_filenames_[id] == ref_filename) {
				ref_module = mod.Get(isolate);
				found = true;
				break;
			}
		}
		if (!found) {
			RejectWithError(context, resolver, "Cannot find referrer module: " + w2s(ref_filename));
			return handle_scope.Escape(promise);
		}
	}

	// 5. 解析目标模块路径
	MaybeLocal<Module> maybe_dep_module =
		ResolveCallback(context, specifier, import_attributes, ref_module);

	if (maybe_dep_module.IsEmpty()) {
		RejectWithError(context, resolver, "Failed to resolve dynamic import: " + spec_str);
		return handle_scope.Escape(promise);
	}

	Local<Module> dep_module = maybe_dep_module.ToLocalChecked();

	// 6. 实例化 + 执行
	TryCatch try_catch(isolate);

	if (dep_module->InstantiateModule(context, ResolveCallback).IsNothing()) {
		if (try_catch.HasCaught()) {
			resolver->Reject(context, try_catch.Exception()).ToChecked();
		}
		else {
			RejectWithError(context, resolver, "Failed to instantiate dynamically imported module");
		}
		return handle_scope.Escape(promise);
	}

	MaybeLocal<Value> eval_result = dep_module->Evaluate(context);
	if (eval_result.IsEmpty()) {
		if (try_catch.HasCaught()) {
			resolver->Reject(context, try_catch.Exception()).ToChecked();
		}
		else {
			RejectWithError(context, resolver, "Failed to evaluate dynamically imported module");
		}
		return handle_scope.Escape(promise);
	}

	// 7. 成功：resolve 为模块 namespace 对象
	Local<Value> ns = dep_module->GetModuleNamespace();
	resolver->Resolve(context, ns).ToChecked();

	return handle_scope.Escape(promise);
}
bool ModuleLoader::LoadDomain(const std::wstring& filename)
{
	auto maybe_module = this->LoadModule(filename);
	if (maybe_module.IsEmpty()) {
		printf("Failed to load %s \n", w2s(filename).c_str());
		return false;
	}
	printf("%s has been loaded \n", w2s(filename).c_str());

	Local<Module> module = maybe_module.ToLocalChecked();
	this->InstantiateAndEvaluate(module);

	return true;
}
