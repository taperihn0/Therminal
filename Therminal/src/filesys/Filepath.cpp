#include "Filepath.hpp"

namespace Thr 
{

FilePath::FilePath()
   : _str("")
{}

FilePath::FilePath(const std::string& s)
   : _str(s)
{}

FilePath::FilePath(const char* s)
   : _str(s)
{}

FilePath::FilePath(const FilePath& fp)
   : _str(fp._str)
{}

FilePath::FilePath(FilePath&& fp)
   : _str(std::move(fp._str))
{}

FilePath& FilePath::operator=(const FilePath& fp)
{
   _str = fp._str;
   return *this;
}

FilePath& FilePath::operator=(FilePath&& fp)
{
   _str = std::move(fp._str);
   return *this;
}

bool FilePath::operator==(const FilePath& fp) const
{
   return _str == fp._str;
}

bool FilePath::operator!=(const FilePath& fp) const
{
   return !(*this == fp);
}

std::string& FilePath::toStr()
{
   return _str;
}

const std::string& FilePath::toStr() const
{
   return _str;
}

const char* FilePath::toCStr() const
{
   return _str.c_str();
}

FilePath FilePath::stepInto(const std::string& s) const
{
#if defined(THR_PLATFORM_WINDOWS)
   static constexpr char SystemPathSeparator = '\\';
#else
   static constexpr char SystemPathSeparator = '/';
#endif

   std::stringstream ss;
   ss << _str << SystemPathSeparator << s;
   return static_cast<FilePath>(ss.str());
}

FilePath FilePath::stepInto(const char* s) const
{
   const std::string tmp = s;
   return stepInto(tmp);
}

FilePath FilePath::operator/(const FilePath& fp) const {
   return this->stepInto(fp.toStr());
}

FilePath FilePath::getCurrentDirectory()
{
   return static_cast<FilePath>(std::filesystem::current_path().string());
}

void FilePath::setCurrentDirectory(const FilePath& dir)
{
   THR_ASSERT_NOT(dir.toStr().empty());
   THR_ASSERT(dir != UndefFilePath);

   std::filesystem::current_path(dir.toStr());
}

bool FilePath::isValid() const 
{
   return _str != UndefFilePath._str;
}

std::ostream& operator<<(std::ostream& out, const FilePath& fp)
{
   return out << fp._str;
}

} // namespace Thr
