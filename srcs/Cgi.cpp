/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:37:51 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/07 16:29:07 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "unistd.h"
Cgi::Cgi(std::string header, std::string file_path, Server *serv)
{
	(void)header;
	(void)file_path;
	(void)serv;
	std::cout << "Constructor of cgi called" << std::endl;
	std::cout << std::endl;
	display_special_characters(header);
	std::cout << std::endl << "cgi_path: " << file_path << std::endl;

	//setup env

	//check if body received is of correct size
	//and send said body to std::cout after duping it
	// if GET + query => '?' in URL, which we put in QUERY_STRING env var
	// if POST + body, print it to std::in of cgi
	int body_fd[2];
	pipe(body_fd);
	if (!header.compare(0, 5, "POST "))
	{
		std::string body = get_body(header);
		write(body_fd[1], body.c_str(), body.size());
	}

	//fork and call cgi

	int pipe_fd[2];
	pipe(pipe_fd);
	pid_t pid = fork();
	if (pid == -1)
		throw Webserv::SystemCallException();
	if (!pid)
	{
		char **args = get_execve_args(file_path);
		// args[0] = new char[file_path.size() + 1];
		// strcpy(args[0], file_path.c_str());
		// args[1] = NULL;
		dup2(pipe_fd[1], 1);
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		dup2(body_fd[0], 0);
		close(body_fd[0]);
		close(body_fd[1]);
		execve(args[0], args, NULL);
		perror(args[0]);
		std::cerr << "execve failure args = ";
		for (int index = 0; args[index]; index++)
		{
			std::cerr << args[index] << ", ";
			delete args[index];
		}
		delete args;
		std::cerr << std::endl;
		std::cout << "HTTP/1.1 500 Internal Server Error\n\n";
		exit(1);
	}
	close(body_fd[0]);
	close(body_fd[1]);
	close(pipe_fd[1]);

	//read from std::in to send message back to server

	std::string bufstr;
	char buffer[BUFFER_SIZE + 1] = {0};
	ssize_t valread = read(pipe_fd[0], buffer, BUFFER_SIZE);
	if (valread == -1)
		serv->send_error(500, "500 Internal Server Error");
	bufstr += buffer;
	while (valread == BUFFER_SIZE)
	{
		valread = read(pipe_fd[0], buffer, BUFFER_SIZE);
		if (valread == -1)
			serv->send_error(500, "500 Internal Server Error");
		else if (valread)
		{
			buffer[valread] = '\0';
			bufstr += buffer;
		}
		// std::cout << "valread: " << valread << std::endl;
	}
	close(pipe_fd[0]);
	waitpid(pid, NULL, 0);

	size_t endhead = bufstr.find("\n\n");
	bufstr = bufstr.substr(0, endhead) + "\nReferer: http://localhost/quotes.cgi\r\n" + bufstr.substr(endhead);
	std::cout << "RETURN CGI:\n" + bufstr;

	//TODO parsing of read input

	if (bufstr.compare(0, 9, "HTTP/1.1 "))
		serv->send_error(500, "500 Internal Server Error");
	std::istringstream iss(bufstr.substr(9, 3));
	int toint;
	iss >> toint;
	if (iss.fail())
		serv->send_error(500, "500 Internal Server Error");
	if (is_error_code(toint))
		serv->send_error(toint, bufstr.substr(9, bufstr.find('\n', 9) - 9));

	// serv->send_message("200 OK");
	send(serv->_socket_fd, bufstr.c_str(), bufstr.size(), 0);

	throw Webserv::QuickerReturnException(); 
}

Cgi::~Cgi(void)
{
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //
