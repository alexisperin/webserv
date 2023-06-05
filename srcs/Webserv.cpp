/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:24:21 by aperin            #+#    #+#             */
/*   Updated: 2023/06/01 15:00:56 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

Webserv::Webserv(void)
{
	
}

Webserv::~Webserv(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
		delete *it;
	this->_servers.clear();
	// std::cout << "Destructor of WebServer called." << std::endl;
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //


// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Webserv::init(std::string file_name)
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
			{
				in_server_block = true;
				this->_servers.push_back(new Server());
			}
			else if (in_server_block)
			{
				if (!line.compare("}"))
				{
					in_server_block = false;
					this->_servers.back()->check_set_default();
				}
				else
					this->_servers.back()->compare_block_info(line, indata);
			}
			else if (line[0] != '#')
				throw Webserv::InvalidFileContentException();
		}
	}
	indata.close();
	if (in_server_block)
		throw Webserv::InvalidFileContentException();
	std::set<int> all_ports;
	size_t number_of_ports = 0;

	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
		(*it)->add_ports(all_ports, &number_of_ports);
}

void Webserv::display_servs_content(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
	{
		(*it)->display_serv_content();
		std::cout << std::endl << std::endl;
	}
}

void Webserv::setup_servers(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	
	pid_t pid;
	for (; it != ite; it++)
	{
		pid = fork();
		if (pid == -1)
		{
			perror("fork");
			return ;
		}
		if (!pid)
		{
			(*it)->setup_server();
			return ;
		}
	}

	it = this->_servers.begin();
	ite = this->_servers.end();
	
	for (; it != ite; it++)
		(*it)->waitup_server();
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

const char* Webserv::MissingDefault404Exception::what() const throw()
{
	return ("[Webserv::MissingDefault404Exception] No default 404 file in root provided by conf file.");
}

const char* Webserv::SystemCallException::what() const throw()
{
	return ("[Webserv::SystemCallException] System call did not call.");
}

const char* Webserv::QuickerReturnException::what() const throw()
{
	return ("[Webserv::QuickerReturnException] gota go fast.");
}
