/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:56:48 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/23 15:45:04 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Webserv.hpp"

Server::Server(void) : _body_size(1)
{
}

Server::~Server(void)
{
	this->_ports.clear();
	this->_server_names.clear();
	this->_index_files.clear();
	this->_error_map.clear();
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //


// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

static int is_error_code(int code)
{
	return ((code >= 400 && code <= 418) || (code >= 421 && code <= 426) || (code >= 428 && code <= 431) || code == 451
		|| (code >= 500 && code <= 508) || (code >= 510 && code <= 511));
}

void Server::compare_block_info(std::string line)
{
	std::cout << "line: " << line << std::endl;
	if (line.back() != ';' || line.find(';') != line.size() - 1)
		throw Webserv::InvalidFileContentException();
	if (!line.compare(0, 7, "listen "))
	{
		std::istringstream iss(line.substr(7));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->_ports.push_back(toint);
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
		line = line.substr(12);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_server_names.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_server_names.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
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
	else if (!line.compare("client_max_body_size 0;") || !line.compare("client_max_body_size 0 ;"))
		this->_body_size = 0;
	else if (!line.compare(0, 21, "client_max_body_size "))
	{
		std::istringstream iss(line.substr(21));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->_body_size = toint;
		size_t index = 21;
		while (std::isdigit(line[index]))
			++index;
		if ((index == line.size() - 2 && line[index] == 'M') || (index == line.size() - 3 && line[index] == 'M' && line[index + 1] == ' '))
			return ;
		throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 11, "error_page "))
	{
		std::list<int> toints;
		size_t index = 11;
		
		while (std::isdigit(line[index]))
		{
			std::istringstream iss(line.substr(index));
			int toint;
			iss >> toint;
			if (iss.fail())
				throw Webserv::InvalidFileContentException();
			if (!is_error_code(toint))
				break ;
			toints.push_back(toint);
			while (std::isdigit(line[index]))
				++index;
			if (line[index] != ' ')
				throw Webserv::InvalidFileContentException();
			++index;
		}
		if (index == 11)
			throw Webserv::InvalidFileContentException();
		std::string value = line.substr(index, line.size() - index - 1 - (line[line.size() - 2] == ' '));
		if (value.find(' ') != std::string::npos)
			throw Webserv::InvalidFileContentException();
		std::list<int>::iterator it = toints.begin();
		std::list<int>::iterator ite = toints.end();
		for (; it != ite; it++)
			this->_error_map.insert(std::pair<int,std::string>(*it, value));
	}
}

void Server::check_set_default(void)
{
	if (this->_index_files.empty() || this->_ports.empty() || this->_root.empty())
		throw Server::IncompleteServerException();
	if (this->_error_map.empty())
	{
		std::string err404("error_files/404.html");
		this->_error_map.insert(std::pair<int,std::string>(404, err404));
	}
	if (this->_server_type.empty())
		this->_server_type = "default_server";
}

void Server::display_serv_content(void)
{
	std::cout << "Server content:" << std::endl;
	std::cout << "\t-ports: ";
	std::list<int>::iterator iit = this->_ports.begin();
	std::list<int>::iterator iite = this->_ports.end();
	for (; iit != iite; iit++)
		std::cout << *iit << ' ';
	std::cout << std::endl;
	std::cout << "\t-server_type: " << this->_server_type << std::endl;
	std::cout << "\t-server_names: ";
	std::list<std::string>::iterator it = this->_server_names.begin();
	std::list<std::string>::iterator ite = this->_server_names.end();
	for (; it != ite; it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "\t-root: " << this->_root << std::endl;
	std::cout << "\t-index: ";
	it = this->_index_files.begin();
	ite = this->_index_files.end();
	for (; it != ite; it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "\t-body_size: " << this->_body_size << std::endl;
	std::cout << "\t-error_codes: ";
	std::map<int, std::string>::iterator iiit = this->_error_map.begin();
	std::map<int, std::string>::iterator iiite = this->_error_map.end();
	for (; iiit != iiite; iiit++)
		std::cout << iiit->first << ' ' << iiit->second << ' ';
	std::cout << std::endl;
}

void Server::add_ports(std::set<int> &all_ports, size_t *number_of_ports)
{
	*number_of_ports += this->_ports.size();
	std::list<int>::iterator it = this->_ports.begin();
	std::list<int>::iterator ite = this->_ports.end();
	for (; it != ite; it++)
		all_ports.insert(*it);
}

// ************************************************************************** //
//                                 Exceptions                                 //
// ************************************************************************** //

const char* Server::IncompleteServerException::what() const throw()
{
	return ("[Server::IncompleteServerException] Missing line in server block.");
}
