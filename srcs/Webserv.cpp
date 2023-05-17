/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:24:21 by aperin            #+#    #+#             */
/*   Updated: 2023/05/17 16:20:05 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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

Webserv::Webserv(std::string file_name) : _body_size(1)
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
	if (this->_index_files.empty() || this->_ports.empty() || this->_root.empty())
		throw Webserv::InvalidFileContentException();
}

Webserv::~Webserv(void)
{
	this->_server_names.clear();
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

static int is_error_code(int code)
{
	return ((code >= 400 && code <= 418) || (code >= 421 && code <= 426) || (code >= 428 && code <= 431) || code == 451
		|| (code >= 500 && code <= 508) || (code >= 510 && code <= 511));
}

void Webserv::compare_block_info(std::string line)
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

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Webserv::display_serv_content(void)
{
	std::cout << "Webserv content:" << std::endl;
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

void Webserv::setup_server(void)
{
	std::list<int>::iterator it = this->_ports.begin();
	std::list<int>::iterator ite = this->_ports.end();
	
	pid_t pid;
	for (; it != ite; it++)
	{
		if (++it != ite)
		{	
			if ((pid = fork()) == -1)
			{
				perror("fork");
				return ;
			}
		}
		else
			pid = 0;
		--it;
		if (!pid)
		{
			std::ifstream indata(this->_index_files.front());
			std::string file_content = "";
			if (!indata.is_open())
				throw Webserv::InvalidFileException();
			std::string line;
			while (!indata.eof()) {
				std::getline( indata, line );
				file_content += line + '\n';
			}
			indata.close();

			int server_fd, new_socket;
			ssize_t valread;
			struct sockaddr_in address;
			int addrlen = sizeof(address);

			// std::string hello("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!");
			std::string content("HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ");
			content += std::to_string(file_content.size()) + "\n\n" + file_content;

			if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
			{
				perror("socket");
				return ;
			}

			address.sin_family = PF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(*it);

			if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
			{
				perror("bind");
				return ;
			}
			if (listen(server_fd, 10) < 0)
			{
				perror("listen");
				return ;
			}

			while(1)
			{
				std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
				if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0)
				{
					perror("accept");
					return ;
				}

				char buffer[30000] = {0};
				valread = recv(new_socket, buffer, 30000, 0);
				std::cout << buffer << std::endl;
				send(new_socket, content.c_str(), content.size(), 0);
				std::cout << "------------------content message sent-------------------\n";
				close(new_socket);
			}
		}
	}
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
