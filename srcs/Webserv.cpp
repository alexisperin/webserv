/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:24:21 by aperin            #+#    #+#             */
/*   Updated: 2023/05/17 11:59:31 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <fstream>
#include <sstream>

static std::string	trim_spaces(std::string str)
{
	int index = 0;
	std::string new_string;
	while (str[index] == ' ' || str[index] == '\t')
		++index;
	while (str[index])
	{
		if (str[index] == '\t')
			new_string += ' ';
		else
			new_string += str[index];
		if (str[index] == ' ' || str[index] == '\t')
		{
			while (str[index] == ' ' || str[index] == '\t')
				++index;
			--index;
		}
		++index;
	}
	if (new_string.back() == ' ')
		new_string.pop_back();
	return (new_string);
}

Webserv::Webserv(std::string file_name)
{
	if (file_name.size() < 6)
		throw Webserv::InvalidFileExtensionException();
	std::string end_name = file_name.substr(file_name.size() - 5);
	if (end_name.compare(".conf"))
		throw Webserv::InvalidFileExtensionException();
	std::ifstream indata(file_name.c_str());
	if (!indata.is_open())
		throw Webserv::InvalidFileException();
	std::string line;
	bool in_server_block = false;
	while (!indata.eof())
	{
		std::getline(indata, line);
		// std::cout << "line: " << line << std::endl;
		line = trim_spaces(line);
		if (!line.empty())
		{
					// std::cout << in_server_block << "comp :" << !line.compare("server {") << ", line: |" << line << '|' << std::endl;
			if (!in_server_block && !line.compare("server {"))
				in_server_block = true;
			else if (in_server_block)
			{
				if (!line.compare("}"))
					in_server_block = false;
				else
					compare_block_info(line);
			}
		}
	}
	indata.close();
}

Webserv::~Webserv(void)
{
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

void Webserv::compare_block_info(std::string line)
{
	std::cout << "line: " << line << std::endl;
	if (line.back() != ';')
		throw Webserv::InvalidFileContentException();
	if (!line.compare(0, 7, "listen "))
	{
		std::istringstream iss(line.substr(7));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->_port = toint;
		size_t index = 7;
		while (std::isdigit(line[index]))
			++index;
		if (index == line.size() - 1 || (index == line.size() - 2 && line[index] == ' '))
			return ;
		if (line.compare(index, 15, " default_server"))
			throw Webserv::InvalidFileContentException();
		this->_server_type = "default_server";
	}
	else if (!line.compare(0, 12, "server_name "))
	{
		
	}
}

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Webserv::display_serv_content(void)
{
	std::cout << "Webserv content:" << std::endl;
	std::cout << "\t-port: " << this->_port << std::endl;
	std::cout << "\t-server_type: " << this->_server_type << std::endl;
}

// ************************************************************************** //
//                                 Exceptions                                 //
// ************************************************************************** //

const char* Webserv::InvalidFileException::what() const throw()
{
	return ("[Webserv::InvalidFileException] Configuration file could not be opened.");
}

const char* Webserv::InvalidFileExtensionException::what() const throw()
{
	return ("[Webserv::InvalidFileExtensionException] Configuration file does not end with '.conf'.");
}

const char* Webserv::InvalidFileContentException::what() const throw()
{
	return ("[Webserv::InvalidFileContentException] Configuration file contains invalid server info.");
}
