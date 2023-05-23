/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:24:21 by aperin            #+#    #+#             */
/*   Updated: 2023/05/23 15:49:41 by yhuberla         ###   ########.fr       */
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
					this->_servers.back()->compare_block_info(line);
			}
		}
	}
	indata.close();
	std::set<int> all_ports;
	size_t number_of_ports = 0;

	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
		(*it)->add_ports(all_ports, &number_of_ports);
	if (number_of_ports != all_ports.size())
		throw Webserv::DuplicatePortsException();
}

Webserv::~Webserv(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
		delete *it;
	this->_servers.clear();
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //


// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

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

void Webserv::setup_server(void)
{
	// struct pollfd pfds[this->_ports.size()]; //to be this->_servers.size()

	// std::list<int>::iterator it = this->_ports.begin();
	// std::list<int>::iterator ite = this->_ports.end();
	
	// for (; it != ite; it++)
	// {
	// 		int server_fd, new_socket;
	// 		ssize_t valread;
	// 		struct sockaddr_in address;
	// 		int addrlen = sizeof(address);


	// 		if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
	// 		{
	// 			perror("socket");
	// 			return ;
	// 		}

	// 		address.sin_family = PF_INET;
	// 		address.sin_addr.s_addr = INADDR_ANY;
	// 		address.sin_port = htons(*it);

	// 		if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
	// 		{
	// 			perror("bind");
	// 			return ;
	// 		}
		
			// std::ifstream indata(this->_index_files.front());
			// std::string file_content = "";
			// if (!indata.is_open())
			// 	throw Webserv::InvalidFileException();
			// std::string line;
			// while (!indata.eof()) {
			// 	std::getline( indata, line );
			// 	file_content += line + '\n';
			// }
			// indata.close();

			// std::string content("HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ");
			// content += std::to_string(file_content.size()) + "\n\n" + file_content;

			// if (listen(server_fd, 10) < 0)
			// {
			// 	perror("listen");
			// 	return ;
			// }

			// while(1)
			// {
			// 	std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
			// 	if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0)
			// 	{
			// 		perror("accept");
			// 		return ;
			// 	}

			// 	char buffer[30000] = {0};
			// 	valread = recv(new_socket, buffer, 30000, 0);
			// 	std::cout << buffer << std::endl;
			// 	send(new_socket, content.c_str(), content.size(), 0);
			// 	std::cout << "------------------content message sent-------------------\n";
			// 	close(new_socket);
			// }
	// }
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

const char* Webserv::DuplicatePortsException::what() const throw()
{
	return ("[Webserv::DuplicatePortsException] At least one port is used multiple times.");
}
