#include <v8.h>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include "file.h"
#include "io.h"

void IO::Register()
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8pp::module io(isolate);

	v8pp::class_<File> file_class(isolate);

	file_class
		.ctor<std::string>() // new File(path) 也可以是wstring
		.function("readAllText", &File::read_all_text)
		.function("readAllBytes", &File::read_all_bytes)
		.function("writeAllText", &File::write_all_text)
		.function("writeAllBytes", &File::write_all_bytes)
		.function("appendText", &File::append_text)
		.function("appendBytes", &File::append_bytes)
		.function("readLines", &File::read_lines)
		.function("writeLine", &File::write_line)
		.function("appendLine", &File::append_line)
		.function("exists", &File::exists)
		.function("remove", &File::remove)
		.function("path", &File::path);

	io.class_("File", file_class);

	isolate->GetCurrentContext()->Global()->Set(context, v8pp::to_v8(isolate, "io"), io.new_instance());
}