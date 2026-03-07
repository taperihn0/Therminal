#pragma once

#include "Filepath.hpp"
#include "logger/Log.hpp"
#include <fstream>
#include <sstream>

namespace Thr
{

THR_INTERNAL std::string readFile(const FilePath& fp)
{
	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	std::string cont;

	try {
		file.open(fp.toStr());

		std::stringstream ss;
		ss << file.rdbuf();

		file.close();

		cont = ss.str();
	}
	catch (const std::ifstream::failure& e) {
		THR_LOG_ERROR("Failed to read file: {}, err: {}", fp.toStr(), e.what());
		return "";
	}

	return cont;
}

} // namespace Thr
