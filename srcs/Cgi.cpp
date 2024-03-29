/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:37:51 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/12 17:01:06 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "unistd.h"
Cgi::Cgi(std::string header, std::string file_path, Server *serv, std::string saved_root)
		: _header(header), _file_path(file_path), _serv(serv)
{
	std::cout << "Constructor of cgi called" << std::endl << std::endl;
	// display_special_characters(header);
	// std::cout << std::endl << "cgi_path: " << file_path << std::endl;

	//check if body received is of correct size
	//and send said body to std::cout after duping it
	// if GET + query => '?' in URL, which we put in QUERY_STRING env var
	// if POST + body, print it to std::in of cgi
	int body_fd[2];
	if (pipe(body_fd) == -1)
		serv->send_error(500, "500 Internal Server Error");
	size_t index = header.find("Content-Length: ");
	size_t expected_size = std::string::npos;
	if (index == std::string::npos)
	{
		if (!header.compare(0, 5, "POST "))
			serv->send_error(411, "411 Length Required");
	}
	else
	{
		std::istringstream iss(header.substr(index + 16, header.find('\n', index + 16)));
		iss >> expected_size;
		if (iss.fail())
			serv->send_error(412, "412 Precondition Failed");
	}
	if (expected_size != std::string::npos)
	{
		if (expected_size > serv->_current_body_size * 1000000)
			serv->send_error(413, "413 Payload Too Large");
		std::string body = get_body(header);
		std::cout << "body size: " << body.size() << " vs expected size: " << expected_size << std::endl;
		if (body.size() != expected_size)
			serv->send_error(412, "412 Precondition Failed");
		write(body_fd[1], body.c_str(), body.size());
	}

	//fork and call cgi

	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		serv->send_error(500, "500 Internal Server Error");
	pid_t pid = fork();
	if (pid == -1)
		serv->send_error(500, "500 Internal Server Error");
	if (!pid)
	{
		//setup env
		char **envp = set_envp(saved_root);
		for (int index = 0; envp[index]; index++)
				std::cout << "env line: " << envp[index] << std::endl;

		char **args = get_execve_args();
		if (dup2(pipe_fd[1], 1) == -1)
			serv->send_error(500, "500 Internal Server Error");
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		if (dup2(body_fd[0], 0) == -1)
			serv->send_error(500, "500 Internal Server Error");
		close(body_fd[0]);
		close(body_fd[1]);
		execve(args[0], args, envp);
		perror(args[0]);
		std::cerr << "execve failure args = ";
		for (int index = 0; args[index]; index++)
		{
			std::cerr << args[index] << ", ";
			delete [] args[index];
		}
		delete [] args;
		for (int index = 0; envp[index]; index++)
			delete [] envp[index];
		delete [] envp;
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

	std::cout << "RETURN CGI:\n" + bufstr;

	//parsing of read input

	if (bufstr.compare(0, 9, "HTTP/1.1 "))
		serv->send_error(500, "500 Internal Server Error");
	std::istringstream iss(bufstr.substr(9, 3));
	int toint;
	iss >> toint;
	if (iss.fail())
		serv->send_error(500, "500 Internal Server Error");
	if (is_error_code(toint))
		serv->send_error(toint, bufstr.substr(9, bufstr.find('\n', 9) - 9));

	send(serv->_socket_fd, bufstr.c_str(), bufstr.size(), 0);

	throw Webserv::QuickerReturnException(); 
}

Cgi::~Cgi(void)
{
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

char **Cgi::get_execve_args(void)
{
	size_t size = this->_file_path.size();
	if (size > 3)
	{
		if (!this->_file_path.compare(size - 3, 3, ".py"))
		{
			char **res = new char *[3];
			res[0] = ft_strdup("/usr/bin/python3");
			res[1] = ft_strdup(this->_file_path.c_str());
			res[2] = NULL;
			return (res);
		}
		if (!this->_file_path.compare(size - 3, 3, ".pl"))
		{
			char **res = new char *[3];
			res[0] = ft_strdup("/usr/bin/perl");
			res[1] = ft_strdup(this->_file_path.c_str());
			res[2] = NULL;
			return (res);
		}
	}
	char **res = new char *[2];
	res[0] = ft_strdup(this->_file_path.c_str());
	res[1] = NULL;
	return (res);
}

char **Cgi::set_envp(std::string saved_root)
{
	/*
    SERVER_PROTOCOL: HTTP/version.
    SERVER_PORT: TCP port (decimal).
    REQUEST_METHOD: name of HTTP method (see above).
    PATH_INFO: path suffix, if appended to URL after program name and a slash. -> header: METHOD saved_root+PATH_INFO PROTOCOL, can be empty!
    PATH_TRANSLATED: corresponding full path as supposed by server. == saved_root+PATH_INFO
    SCRIPT_NAME: relative path to the program, like /cgi-bin/script.cgi. -> file_path - saved_root
    REMOTE_HOST: host name of the client, unset if server did not perform such lookup.
*   //REMOTE_ADDR: IP address of the client (dot-decimal). -> getaddrinfo ?

    SERVER_NAME: host name of the server, may be dot-decimal IP address. -> serv->serv_names w/ ':'
    CONTENT_TYPE: Internet media type of input data if PUT or POST method are used, as provided via HTTP header. -> Sec-Fetch-Dest
    CONTENT_LENGTH: similarly, size of input data (decimal, in octets) if provided via HTTP header.
	HTTP_ACCEPT
	HTTP_ACCEPT_LANGUAGE
	HTTP_USER_AGENT
	QUERY_STRING: the part of URL after the "?" character. The query string may be composed of *name=value pairs separated with ampersands (such as var1=val1&var2=val2...) when used to submit form data transferred via GET method as defined by HTML application/x-www-form-urlencoded.
	
 *	//HTTP_COOKIE for now
	*/

	std::map<std::string, std::string> env_map;
	env_map.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
	env_map.insert(std::pair<std::string, std::string>("SERVER_PORT", get_port()));
	env_map.insert(std::pair<std::string, std::string>("REQUEST_METHOD", get_method()));
	// env_map.insert(std::pair<std::string, std::string>("PATH_INFO", 
	add_path_info(env_map, saved_root);
	// env_map.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", get_path_translated()));
	env_map.insert(std::pair<std::string, std::string>("SCRIPT_NAME", get_script_relative(saved_root)));
	env_map.insert(std::pair<std::string, std::string>("REMOTE_HOST", get_remote_host()));
	add_server_names(env_map);
	add_header_field(env_map, "CONTENT_TYPE", "Content-Type: ");
	add_header_field(env_map, "CONTENT_LENGTH", "Content-Length: ");
	add_header_field(env_map, "HTTP_ACCEPT", "Accept: ");
	add_header_field(env_map, "HTTP_ACCEPT_LANGUAGE", "Accept-Language: ");
	add_header_field(env_map, "HTTP_USER_AGENT", "User-Agent: ");
	add_header_field(env_map, "HTTP_COOKIE", "Cookie: ");

	return (map_to_array(env_map));
}

std::string Cgi::get_port(void)
{
	size_t index_start = this->_header.find("Host: ");
	size_t index_end = this->_header.find('\n', index_start);

	std::string host_line = this->_header.substr(index_start, index_end - index_start);
	size_t index_port = host_line.find(':', 5);
	if (index_port == std::string::npos)
		return ("80");
	return (host_line.substr(index_port + 1, host_line.size() - (index_port + 1) - (host_line[host_line.size() - 1] == '\r')));
}

std::string Cgi::get_method(void)
{
	size_t index = this->_header.find(' ');
	return (this->_header.substr(0, index));
}

void Cgi::add_path_info(std::map<std::string, std::string> & env_map, std::string root)
{
	size_t index_start = this->_header.find(' ') + 1;
	size_t index_end = this->_header.find(' ', index_start);
	size_t index_query = this->_header.find('?', index_start);
	if (index_query != std::string::npos && index_query < index_end)
	{
		std::string query_string = this->_header.substr(index_query + 1, index_end - (index_query + 1));
		env_map.insert(std::pair<std::string, std::string>("QUERY_STRING", query_string));
		index_end = index_query;
	}
	std::string path_info = this->_header.substr(index_start + root.size(), index_end - (index_start + root.size()));
	std::string path_translated = this->_header.substr(index_start, index_end - index_start);
	env_map.insert(std::pair<std::string, std::string>("PATH_INFO", path_info));
	env_map.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", path_translated));
}

std::string Cgi::get_script_relative(std::string root)
{
	return (this->_file_path.substr(root.size(), this->_file_path.size() - root.size()));
}

std::string Cgi::get_remote_host(void)
{
	size_t index_start = this->_header.find("Host: ");
	size_t index_end = this->_header.find('\n', index_start);

	std::string host_line = this->_header.substr(index_start, index_end - index_start);
	size_t index_port = host_line.find(':', 5);
	if (index_port == std::string::npos)
		return (host_line.substr(6, host_line.size() - 6 - (host_line[host_line.size() - 1] == '\r')));
	return (host_line.substr(6, index_port - 6));
}

void Cgi::add_server_names(std::map<std::string, std::string> & env_map)
{
	std::list<std::string>::iterator it = this->_serv->_server_names.begin();
	std::list<std::string>::iterator ite = this->_serv->_server_names.end();

	if (it == ite)
		return ;

	std::string names = *it;
	it++;
	for (; it != ite; it++)
	{
		names += ':' + *it;
	}
	env_map.insert(std::pair<std::string, std::string>("SERVER_NAME", names));
}

void Cgi::add_header_field(std::map<std::string, std::string> & env_map, std::string key, std::string header_key)
{
	size_t index_start = this->_header.find(header_key);

	if (index_start == std::string::npos)
		return ;

	size_t index_end = this->_header.find('\n', index_start);


	env_map.insert(std::pair<std::string, std::string>(key, this->_header.substr(index_start + header_key.size(), index_end - (index_start + header_key.size()) - (this->_header[index_end - 1] == '\r'))));
}

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //
