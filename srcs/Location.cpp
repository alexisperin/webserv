/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/25 13:12:38 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/25 18:18:30 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(std::string line, std::ifstream & indata, std::string root) : _auto_index(true), _auto_sighted(false),
	_line_sighted(false), _return_sighted(false), _root(root)
{
	if (root.empty())
		throw Webserv::InvalidFileContentException();
	size_t index = line.find(' ', 9);
	if (line.compare(index + 1, 2, "{"))
		throw Webserv::InvalidFileContentException();
	this->_location = line.substr(9, line.find(' ', 9) - 9);
	if (this->_location.empty() || this->_location[0] != '/')
		throw Webserv::InvalidFileContentException();
	while (!indata.eof())
	{
		std::getline(indata, line);
		line = trim_spaces(line);
		if (!line.empty())
		{
			if (!line.compare("}"))
			{
				check_set_default();
				std::cout << "LOCATION: " << this->_location << std::endl;
				return ;
			}
			else
			{
				if (this->_return_sighted)
					throw Webserv::InvalidFileContentException();
				compare_block_info(line);
				this->_line_sighted = true;
			}
		}
	}
	throw Webserv::InvalidFileContentException();
}

Location::~Location(void)
{

}

Location &Location::operator=(const Location & other)
{
	this->_return = other._return;
	this->_methods = other._methods;
	this->_auto_index = other._auto_index;
	this->_root = other._root;
	this->_index_files = other._index_files;
	return (*this);
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

void Location::compare_block_info(std::string line)
{
	std::cout << "line: " << line << std::endl;
	if (line[0] == '#')
		;
	else if (line.back() != ';' || line.find(';') != line.size() - 1)
		throw Webserv::InvalidFileContentException();
	else if (!line.compare(0, 5, "root "))
	{
		this->_root = line.substr(5, line.size() - 6 - (line[line.size() - 2] == ' '));
		if (this->_root.find(' ') != std::string::npos || !this->_root.compare(";"))
			throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 6, "index "))
	{
		line = line.substr(6);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_index_files.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_index_files.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare(0, 10, "autoindex "))
	{
		if (this->_auto_sighted)
			throw Webserv::InvalidFileContentException();
		this->_auto_sighted = true;
		line = line.substr(10, line.size() - 11 - (line[line.size() - 2] == ' '));
		if (line.find(' ') != std::string::npos || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		if (!line.compare("on"))
			this->_auto_index = true;
		else if (!line.compare("off"))
			this->_auto_index = false;
		else
			throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 14, "allow_methods "))
	{
		line = line.substr(14);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_methods.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_methods.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare(0, 7, "return "))
	{
		if (this->_line_sighted)
			throw Webserv::InvalidFileContentException();
		this->_return = line.substr(7, line.size() - 8 - (line[line.size() - 2] == ' '));
		if (this->_return.find(' ') != std::string::npos || !this->_return.compare(";"))
			throw Webserv::InvalidFileContentException();
		this->_return_sighted = true;
	}
}

void Location::check_set_default(void)
{
	if (!this->_return.empty())
		return ;
	if (this->_root.empty())
		throw Webserv::InvalidFileContentException();
	for (size_t index = 0; index < this->_methods.size(); index++)
	{
		for (size_t sub_index = 0; sub_index < index; sub_index++)
		{
			if (this->_methods[index] == this->_methods[sub_index])
				throw Webserv::InvalidFileContentException();
		}
		if (this->_methods[index] != "GET" && this->_methods[index] != "PUT" && this->_methods[index] != "POST"
			&& this->_methods[index] != "DELETE" && this->_methods[index] != "HEAD")
			throw Webserv::InvalidFileContentException();
	}
	if (this->_methods.empty())
	{
		this->_methods.push_back("GET");
		this->_methods.push_back("PUT");
		this->_methods.push_back("POST");
		this->_methods.push_back("DELETE");
		this->_methods.push_back("HEAD");
	}
	std::sort(this->_methods.begin(), this->_methods.end());
}


// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Location::display_loc_content(void)
{
	std::cout << "\t-location: " << this->_location << std::endl;
	std::cout << "\t  -root: " << this->_root << std::endl;
	std::cout << "\t  -auto_index: ";
	(this->_auto_index)
		? std::cout << "on" << std::endl
		: std::cout << "off" << std::endl;
	std::cout << "\t  -index: ";
	std::list<std::string>::iterator it = this->_index_files.begin();
	std::list<std::string>::iterator ite = this->_index_files.end();
	for (; it != ite; it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "\t  -methods: ";
	std::vector<std::string>::iterator iit = this->_methods.begin();
	std::vector<std::string>::iterator iite = this->_methods.end();
	for (; iit != iite; iit++)
		std::cout << *iit << ' ';
	std::cout << std::endl;
	std::cout << "\t  -return: " << this->_return << std::endl;
}
