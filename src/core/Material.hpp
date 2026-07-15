#ifndef __Material_hpp__
#define __Material_hpp__

#include "Common.hpp"


class Material
{
public:
	Material(const std::string& name);
	virtual ~Material() = default;
	const std::string& get_name() const { return m_name; }
private:
	std::string m_name;
};

#endif
