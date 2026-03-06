#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include"encoding.h"
#include"utils.h"
using namespace utils;

class File
{
public:
	explicit File(std::string path)
		: path_(GetModuleDirectoryUtf8() + std::move(path))
	{
	}

	// 读取整个文件为字符串（文本）
	std::string read_all_text() const
	{
		std::ifstream ifs(s2w(path_), std::ios::binary);
		if (!ifs)
			throw std::runtime_error("Failed to open file for reading: " + path_);

		std::string content;
		ifs.seekg(0, std::ios::end);
		content.resize(static_cast<size_t>(ifs.tellg()));
		ifs.seekg(0, std::ios::beg);
		ifs.read(&content[0], static_cast<std::streamsize>(content.size()));
		return content;
	}

	// 读取整个文件为字节数组（适合二进制）
	std::vector<uint8_t> read_all_bytes() const
	{
		std::ifstream ifs(s2w(path_), std::ios::binary);
		if (!ifs)
			throw std::runtime_error("Failed to open file for reading: " + path_);

		ifs.seekg(0, std::ios::end);
		auto size = static_cast<size_t>(ifs.tellg());
		ifs.seekg(0, std::ios::beg);

		std::vector<uint8_t> data(size);
		ifs.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
		return data;
	}

	// 覆盖写入文本
	void write_all_text(std::string const& text) const
	{
		std::ofstream ofs(s2w(path_), std::ios::binary | std::ios::trunc);
		if (!ofs)
			throw std::runtime_error("Failed to open file for writing: " + path_);

		ofs.write(text.data(), static_cast<std::streamsize>(text.size()));
		if (!ofs)
			throw std::runtime_error("Failed to write file: " + path_);
	}

	// 覆盖写入字节
	void write_all_bytes(std::vector<uint8_t> const& data) const
	{
		std::ofstream ofs(s2w(path_), std::ios::binary | std::ios::trunc);
		if (!ofs)
			throw std::runtime_error("Failed to open file for writing: " + path_);

		ofs.write(reinterpret_cast<char const*>(data.data()),
			static_cast<std::streamsize>(data.size()));
		if (!ofs)
			throw std::runtime_error("Failed to write file: " + path_);
	}

	// 追加文本
	void append_text(std::string const& text) const
	{
		std::ofstream ofs(s2w(path_), std::ios::binary | std::ios::app);
		if (!ofs)
			throw std::runtime_error("Failed to open file for appending: " + path_);

		ofs.write(text.data(), static_cast<std::streamsize>(text.size()));
		if (!ofs)
			throw std::runtime_error("Failed to append file: " + path_);
	}

	// 追加字节
	void append_bytes(std::vector<uint8_t> const& data) const
	{
		std::ofstream ofs(s2w(path_), std::ios::binary | std::ios::app);
		if (!ofs)
			throw std::runtime_error("Failed to open file for appending: " + path_);

		ofs.write(reinterpret_cast<char const*>(data.data()),
			static_cast<std::streamsize>(data.size()));
		if (!ofs)
			throw std::runtime_error("Failed to append file: " + path_);
	}

	// 读取所有行
	std::vector<std::string> read_lines() const
	{
		std::ifstream ifs(s2w(path_));
		if (!ifs)
			throw std::runtime_error("Failed to open file for reading: " + path_);

		std::vector<std::string> lines;
		std::string line;

		while (std::getline(ifs, line))
		{
			// 去掉 Windows 的 \r
			if (!line.empty() && line.back() == '\r')
				line.pop_back();

			lines.push_back(line);
		}

		return lines;
	}

	// 写入一行（覆盖写）
	void write_line(std::string const& line) const
	{
		std::ofstream ofs(s2w(path_), std::ios::trunc);//清空文件
		if (!ofs)
			throw std::runtime_error("Failed to open file for writing: " + path_);

		ofs << line << "\n";
	}

	// 追加一行
	void append_line(std::string const& line) const
	{
		std::ofstream ofs(s2w(path_), std::ios::app);
		if (!ofs)
			throw std::runtime_error("Failed to open file for appending: " + path_);

		ofs << line << "\n";
	}


	// 是否存在
	bool exists() const
	{
		return std::filesystem::exists(s2w(path_));
	}

	// 删除文件
	bool remove() const
	{
		std::error_code ec;
		bool removed = std::filesystem::remove(s2w(path_), ec);
		if (ec)
			throw std::runtime_error("Failed to remove file: " + path_ + " : " + ec.message());
		return removed;
	}

	// 获取路径
	std::string path() const
	{
		return path_;
	}


private:
	std::string path_;
};

